* Reference
* mmap效率高的一个原因是，与read相比，具有更少的系统调用(只用一次)。https://stackoverflow.com/questions/39280264/when-to-use-mmap-vs-when-to-use-read-and-write-with-cache-layer
* mmap的数据还是要经过内核（page cache)，但是，避免了一次copy直接利用虚拟地址访问page cache: https://www.quora.com/How-is-a-mmaped-file-I-O-different-from-a-regular-file-I-O-with-regard-to-the-kernel 而read的话，则需要将读到page cache的数据拷贝到read调用指定的用户内存空间。
* mmap的确定，适用面不如read广泛，mmap基于page fault,如果你的读写场景会引入很多的page fault，性能不一定比read好, page fault也是很消耗时间的操作，
* mmap场景，长时间打开的文件，如数据库文件：
  * Only applications such as databases which open a file and then keep them open for a long long time prefer to use mmap().
