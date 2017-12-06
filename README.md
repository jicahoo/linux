# linux
Learning Linux and OS knowledge

# /proc
* http://advancedlinuxprogramming.com/alp-folder/alp-ch07-proc-filesystem.pdf

# ptrace
* https://www.mkssoftware.com/docs/man3/wait3.3.asp
* http://blog.csdn.net/edonlii/article/details/8717029



## 如何透彻理解epoll.
* epoll主要是在server端，更高效地（省CPU和线程资源）处理SocketIO, 对TCP协议而言，就是透明的。
* 基础是你要理解Linux的文件描述符，Socket编程的细节。
* 你也要理解epoll之前的解决方案：select, <深入理解计算机系统>介绍的还算可以。
* 在Java NIO中，他们为什么称之为面向Buffer的数据传输，因为，是基于事件的，当可读事件到达时，已经有一块数据从网络中到达了你的机器。 Select又对应谁？Select对应epoll_create1返回的epoll文件描述符。
* https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
## epoll在不同语言里面的体现
* http://python.jobbole.com/88291/ Python 深入理解 Python 异步编程
* http://www.ece.virginia.edu/cheetah/documents/papers/TCPlinux.pdf TCP implementation in Linux
