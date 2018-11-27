# linux
Learning Linux and OS knowledge

# 方法论
* 学习Linux的某个功能，可以从那些角度去学习呢？
 * 内核的相关运行机制,
 * 相关的系统调用,libc相关的函数
 * 编程使用相关功能,
 * 使用工具观察内核功能相关信息。/proc ....
 
# Q&A
* 多进程写文件，会有并发问题吗？有，参见，<Unix环境高级编程> 3.1.1 原子操作。
* 为什么VSS会增长? 

# /proc
* http://advancedlinuxprogramming.com/alp-folder/alp-ch07-proc-filesystem.pdf


## ptrace
* https://www.mkssoftware.com/docs/man3/wait3.3.asp
* http://blog.csdn.net/edonlii/article/details/8717029

## strace
* 对应Unix-like的truss.
* 可以查看应用程序相关的系统调用。在研究Go语言的socket编程的时候，用到了，发现某个LWP会停在epoll_wait(4处。类似如下输出:
```bash
stack@ubuntu16-dev:~/go$ sudo strace -p 30412
strace: Process 30412 attached
epoll_wait(4,
```
* strace是基于ptrace系统调用开发的。

## ps
* 列出轻量级线程：ps -p 30408 -L

## 如何透彻理解epoll.
* epoll主要是在server端，更高效地（省CPU和线程资源）处理SocketIO, 对TCP协议而言，就是透明的。
* 基础是你要理解Linux的文件描述符，Socket编程的细节。
* Socket描述符，设置成非阻塞，意味着什么？https://www.scottklement.com/rpg/socktut/nonblocking.html blocking vs non blocking io
* TCP在两端是如何实现和交互的。
  * http://www.ece.virginia.edu/cheetah/documents/papers/TCPlinux.pdf TCP implementation in Linux

* 你也要理解epoll之前的解决方案：select, <深入理解计算机系统>介绍的还算可以。
* 在Java NIO中，他们为什么称之为面向Buffer的数据传输，因为，是基于事件的，当可读事件到达时，已经有一块数据从网络中到达了你的机器。 Select又对应谁？Select对应epoll_create1返回的epoll文件描述符。
* https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/

## epoll在不同语言里面的体现
* http://python.jobbole.com/88291/ Python 深入理解 Python 异步编程 英文源：http://aosabook.org/en/500L/a-web-crawler-with-asyncio-coroutines.html
* https://morsmachine.dk/netpoller Go Lang netpoller. 实现更炫。你要理解Go语言是如何调度go routine的。https://www.bbsmax.com/A/x9J2xjPMd6/ Go并发小结。
* JAVA NIO
* Netty (Netty has native epoll)
* Scala: https://ci.apache.org/projects/flink/flink-docs-release-1.2/dev/stream/asyncio.html Flick aysnc io.http://wuchong.me/blog/2017/05/17/flink-internals-async-io/

##　Go netpoller
* https://stackoverflow.com/questions/36112445/golang-blocking-and-non-blocking

## Python 3.5 asyncio
* Code. see python-echo-server-asyncio.py

* strace result. Saw epoll_wait, mremap. 
```bash
sudo strace -p 8494  #8489 is the pid of python.
[sudo] password for jichao: 
strace: Process 8494 attached
epoll_wait(3, [{EPOLLIN, {u32=7, u64=3359015547638382599}}], 3, -1000) = 1
mmap(NULL, 266240, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f092accc000
recvfrom(7, "hello\r\n", 262144, 0, NULL, NULL) = 7
mremap(0x7f092accc000, 266240, 4096, MREMAP_MAYMOVE) = 0x7f092accc000
write(2, "EchoServer_127.0.0.1_46318: rece"..., 50) = 50
sendto(7, "hello\r\n", 7, 0, NULL, 0)   = 7
write(2, "EchoServer_127.0.0.1_46318: sent"..., 46) = 46
munmap(0x7f092accc000, 4096)            = 0
epoll_wait(3, [{EPOLLIN, {u32=7, u64=3359015547638382599}}], 3, -1000) = 1
mmap(NULL, 266240, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f092accc000
recvfrom(7, "Nihao\r\n", 262144, 0, NULL, NULL) = 7
mremap(0x7f092accc000, 266240, 4096, MREMAP_MAYMOVE) = 0x7f092accc000
write(2, "EchoServer_127.0.0.1_46318: rece"..., 50) = 50
sendto(7, "Nihao\r\n", 7, 0, NULL, 0)   = 7
write(2, "EchoServer_127.0.0.1_46318: sent"..., 46) = 46
munmap(0x7f092accc000, 4096)            =

```


## Linux device file
* https://askubuntu.com/questions/397493/what-does-the-first-character-of-unix-mode-string-indicate
* https://www.cyberciti.biz/faq/understanding-unix-linux-bsd-device-files/
* https://unix.stackexchange.com/questions/18239/understanding-dev-and-its-subdirs-and-files/18534#18534
* http://www.tldp.org/LDP/lkmpg/2.4/html/c577.htm

