#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <limits.h>

#define MAXEVENTS 64


char* int2bin(int i)
{
    size_t bits = sizeof(int) * CHAR_BIT;

    char * str = malloc(bits + 1);
    if(!str) return NULL;
    str[bits] = 0;

    // type punning because signed shift is implementation-defined
    unsigned u = *(unsigned *)&i;
    for(; bits--; u >>= 1)
        str[bits] = u & 1 ? '1' : '0';

    return str;
}


//Reference: https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
static int make_socket_non_blocking (int sfd) {
    int flags, s;

    flags = fcntl (sfd, F_GETFL, 0);
    if (flags == -1) {
        perror ("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl (sfd, F_SETFL, flags);
    if (s == -1)
    {
        perror ("fcntl");
        return -1;
    }

    return 0;
}

//Get port info
in_port_t get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return (((struct sockaddr_in*)sa)->sin_port);
    }

    return (((struct sockaddr_in6*)sa)->sin6_port);
}

/**
 * return a file descriptor poiting to a socket. (*:port LISTEN)
 */
static int create_and_bind (char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, sfd;

    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    s = getaddrinfo (NULL, port, &hints, &result);
    if (s != 0) {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
        return -1;
    } else {
        printf("[OK]getaddrinfo\n");
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) {
            continue;
        } else {
            printf("[OK]Socket fd: %d\n", sfd);
            printf("\tai_family: %d\n", rp->ai_family);
            printf("\tai_socktype: %d\n", rp->ai_socktype);
            printf("\tai_protocol: %d\n", rp->ai_protocol);
            printf("\tport is: %d\n", ntohs(get_in_port((struct sockaddr *)rp->ai_addr)));
        }

        //Port info is in rp->ai_addr. So bind that port...
        s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
        if (s == 0) {
            printf("[OK] Socket Bind successfully.\n");
            break;
        } else {
            // http://students.mimuw.edu.pl/SO/Linux/Kod/include/linux/socket.h.html
            //May try differnt ai_family. IPv4 or IPv6.
            printf("[WARN] Socket bind failed. Error message: %s. Try next one...\n", strerror(errno));
        }


        close (sfd);
    }

    if (rp == NULL) {
        fprintf (stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo (result);

    return sfd;
}

int main (int argc, char *argv[])
{
    int sfd, s;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;

    if (argc != 2) {
        fprintf (stderr, "Usage: %s [port]\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    sfd = create_and_bind (argv[1]);
    if (sfd == -1) {
        abort ();
    }

    s = make_socket_non_blocking(sfd);
    if (s == -1) {
        abort ();
    }

    // The second parameter of listen is backlog.
    // The backlog argument defines the maximum length to which the queue of pending connections for sockfd may grow.
    // There is pending connections queue for socket fd.
    printf("[OK] Max pending connections queue size for socket fd: %d\n", SOMAXCONN);
    s = listen (sfd, SOMAXCONN);
    if (s == -1) {
        perror ("listen");
        abort ();
    }

    efd = epoll_create1 (0);
    if (efd == -1) {
        perror ("epoll_create");
        abort ();
    } else {
       printf("[OK] Got epoll fd: %d\n", efd); 
    }

    event.data.fd = sfd;
    //EPOLLET means using Edge Triggering.
    event.events = EPOLLIN | EPOLLET;
    printf("[OK] Registser event: \n");
    printf("\tfor socket fd: %d\n", event.data.fd);
    printf("\tregister event: %s\n", int2bin(event.events));
    s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);
    if (s == -1) {
        perror ("epoll_ctl");
        abort ();
    }

    /* Buffer where events are returned */
    printf("[INFO] Max events: %d\n", MAXEVENTS);
    events = calloc (MAXEVENTS, sizeof event);

    /* The event loop */
    int evt_rnd = 0;
    while (1) {
        evt_rnd++;
        int n, i;

        printf("[INFO] [EventRound: %d].\n", evt_rnd);
        n = epoll_wait (efd, events, MAXEVENTS, -1);
        printf("[INFO] Got %d events. [EventRound: %d].\n", n, evt_rnd);
        for (i = 0; i < n; i++) {
            printf("\t[INFO] [EventRound: %d, EventIndex: %d]. \n", evt_rnd, i);
            if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN))) {
                /* An error has occured on this fd, or the socket is not
                   ready for reading (why were we notified then?) */
                fprintf (stderr, "epoll error\n");
                close (events[i].data.fd);
                continue;
            } else if (sfd == events[i].data.fd) {
                int nconn_nr = 0;
                //One event got, but there maybe several pending incoming connections on the sfd.
                //So need a loop for connect them.
                while (1) {
                    nconn_nr++;
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof in_addr;
                    infd = accept (sfd, &in_addr, &in_len);
                    if (infd == -1) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            /* We have processed all incoming
                               connections. */
                            break;
                        } else {
                            perror ("accept");
                            break;
                        }
                    }

                    s = getnameinfo (&in_addr, in_len,
                            hbuf, sizeof hbuf,
                            sbuf, sizeof sbuf,
                            NI_NUMERICHOST | NI_NUMERICSERV);
                    if (s == 0) {
                        printf("\t[INFO] Accepted connection on socket file descriptor %d "
                                "(host=%s, port=%s) [EventRound: %d, NewConnRound: %d]. \n", infd, hbuf, sbuf, evt_rnd, nconn_nr);
                    }

                    /* Make the incoming socket non-blocking and add it to the
                       list of fds to monitor. */
                    s = make_socket_non_blocking (infd);
                    if (s == -1) {
                        printf("\t[ERROR] Failed to make fd non-blocking: %d\n", infd);
                        abort ();
                    }

                    event.data.fd = infd;
                    event.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
                    if (s == -1)
                    {
                        perror ("epoll_ctl");
                        abort ();
                    }
                }
                continue;
            } else {
                /* We have data on the fd waiting to be read. Read and
                   display it. We must read whatever data is available
                   completely, as we are running in edge-triggered mode
                   and won't get a notification again for the same
                   data. */
                int done = 0;

                int rd_rnd=0;
                while (1) {
                    rd_rnd++;
                    printf("\t[INFO]Read socket: %d. [EventRound: %d, ReadSocketRound: %d].\n", events[i].data.fd, evt_rnd, rd_rnd);
                    ssize_t count;
                    char buf[8];

                    count = read (events[i].data.fd, buf, sizeof buf);
                    if (count == -1) {
                        /* If errno == EAGAIN, that means we have read all
                           data. So go back to the main loop. */
                        if (errno != EAGAIN) {
                            perror ("read");
                            done = 1;
                        }
                        break;
                    } else if (count == 0) {
                        /* End of file. The remote has closed the
                           connection. */
                        done = 1;
                        break;
                    }

                    /* Write the buffer to standard output */
                    s = write (1, buf, count);
                    if (s == -1) {
                        perror ("write");
                        abort ();
                    }
                }

                if (done) {
                    printf ("\t[INFO] Closed connection on descriptor %d. [EventRound: %d, ReadSocketRound: %d].\n", events[i].data.fd, evt_rnd, rd_rnd);

                    /* Closing the descriptor will make epoll remove it
                       from the set of descriptors which are monitored. */
                    close (events[i].data.fd);
                }
            }
        }
    }

    free (events);

    close (sfd);

    return EXIT_SUCCESS;
}
