## 
* 操作系统底层，以页为粒度，进行内存的分配和释放。
* malloc, vmalloc, kmalloc, alloc\_page(返回物理页结构体), get\_free\_page(返回页对应的逻辑地址)。这些内存分配相关的函数，有几个维度可以区分：内核还是用户空间，粒度(page还是byte)，返回什么(地址值，物理页结构）
* SLAB就是以页为基础，实现是基于__get_free_pages
* kmalloc利用了SLAB
