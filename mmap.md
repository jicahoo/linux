* Reference
* mmap效率高的一个原因是，与read相比，具有更少的系统调用(只用一次)。https://stackoverflow.com/questions/39280264/when-to-use-mmap-vs-when-to-use-read-and-write-with-cache-layer
* mmap的数据还是要经过内核（page cache): https://www.quora.com/How-is-a-mmaped-file-I-O-different-from-a-regular-file-I-O-with-regard-to-the-kernel
