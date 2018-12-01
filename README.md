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
 * 在Linux操作系统上，任何的内存分配调用最终都会落到两个系统调用上：brk 或 mmap. 这两个系统调用分配的是虚拟内存，只有你第一次读写对应虚拟内存区域的时候，就会发生缺页中断，真正分配一个物理页（这个页就会占用RSS）。那么，什么情况下，物理页释放了，但是VSS不会随之下降？
 * brk(negative_num),munmap都应该会释放物理页（去掉虚拟内存和物理页之间的映射)
 * 《Linux内核的设计与实现》第15章，进程地址空间，每个进程都有个进程描述符mm_struct, mm_struct是由vma(虚拟内存区域)组成的。vm_area_struct对应的操作，就有close操作`void close(struct vm_area_struct *area)`，该操作就会将指定的内存区域移除（虚拟）地址空间。munmap就会调用该内核函数，参见：https://github.com/torvalds/linux/blob/master/mm/mmap.c
 * VSS什么情况下会下降？
    * 调用brk(negative_num)
    * munmap
    * 但是，在C语言层面，free的内存是堆上的话，并不一定导致调用brk(negative_num), 而已，也不会去掉对应虚拟地址与物理页的映射，因为操作系统并不知道这个free(没有触发系统调用），free是C语言层面的，malloc会管理堆。只是在系统发现内存紧张的时候，会把这个对应的物理页交换(swap)出去。因为在C语言层面已经释放，不会有新的访问，所以，操作系统会觉察到这个物理页不是活跃的页，就会把它交换出去。如果该物理页在某一时刻，又被加载回来，随后，某个malloc调用，可能会被重新分配到这个虚拟地址和对应物理页，继续使用。Chen 看了malloc源码得到的结论。
    * JVM中reserved size是VSS, 但committed size并不是RSS, committed是JVM层面的统计信息，应用程序本身是无法通过自己计算出RSS, RSS只有操作系统有能力知道。什么是JVM中Commited，我们猜测是如下的解释，JVM可以调用mmap开辟一段虚拟地址空间，假设是10MB, 假设我们把一个大小为int\[1024\*1024\]数组，我们把前512\*1024个元素设置为1, 并放在mmap开辟的10MB空间里面，那么这个情况下，我们用了 VSS  10MB，   Committed 4MB,  RSS: 2MB. 假设一个int占用四个字节。Commited就是JVM层次的统计。 RSS是我们根据操作系统机制推算出的，应用程序并不知道这个信息。Try example to verify it: http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/2-C-adv-data/dyn-array.html 
    * Java Reserved vs Commited: https://stackoverflow.com/questions/31173374/why-does-a-jvm-report-more-committed-memory-than-the-linux-process-resident-set
    * JVM code: https://github.com/unofficial-openjdk/openjdk/blob/531ef5d0ede6d733b00c9bc1b6b3c14a0b2b3e81/src/hotspot/share/services/memReporter.cpp
    * 理解JVM里面一些有关内存的指标，你还需要了解glibc的malloc函数，它是如何为应用程序提供内存分配和释放服务的。OS(brk, mmap) -> glibc (malloc) -> C/C++ (new/delete, 引用计数的对象生命周期管理, GC算法) -> Java (new)。可以继续研究NMT的实现与各项指标。
    * 
    
 * https://stackoverflow.com/questions/561245/virtual-memory-usage-from-java-under-linux-too-much-memory-used/561450#561450
 * https://stackoverflow.com/questions/7880784/what-is-rss-and-vsz-in-linux-memory-management
 * https://blog.holbertonschool.com/hack-the-virtual-memory-malloc-the-heap-the-program-break/
 * https://caomingkai.github.io/2018/04/14/Virtual-Memory-Allocation-brk-and-mmap/
 * https://blog.csdn.net/gfgdsg/article/details/42709943
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

