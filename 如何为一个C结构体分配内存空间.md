## 
* 操作系统底层，以页为粒度，进行内存的分配和释放。
* malloc, vmalloc, kmalloc, alloc\_page(返回物理页结构体), get\_free\_page(返回页对应的逻辑地址)。这些内存分配相关的函数，有几个维度可以区分：内核还是用户空间，粒度(page还是byte)，返回什么(地址值，物理页结构）
* SLAB就是以页为基础，实现是基于__get_free_pages
* kmalloc利用了SLAB

## 参考
* 阐述了SLAB的目标, kmalloc和SLAB的关系: https://www.kernel.org/doc/gorman/html/understand/understand011.html
  * SLAB三个目标：分配小块内存，做Cache, L1/L2 Cache Leverage. 注意第一个目标是分配小块内存，防止内存碎片。
  * kmalloc利用SLAB才得以具有分配任意大小块的能力。
```
The slab allocator has three principle aims:

The allocation of small blocks of memory to help eliminate internal fragmentation that would be otherwise caused by the buddy system;
The caching of commonly used objects so that the system does not waste time allocating, initialising and destroying objects. Benchmarks on Solaris showed excellent speed improvements for allocations with the slab allocator in use [Bon94];
The better utilisation of hardware cache by aligning objects to the L1 or L2 caches.

To help eliminate internal fragmentation normally caused by a binary buddy allocator, two sets of caches of small memory buffers ranging from 25 (32) bytes to 217 (131072) bytes are maintained. One cache set is suitable for use with DMA devices. These caches are called size-N and size-N(DMA) where N is the size of the allocation, and a function kmalloc() (see Section 8.4.1) is provided for allocating them. With this, the single greatest problem with the low level page allocator is addressed. The sizes caches are discussed in further detail in Section ??.
```

* 通用slab和专用slab. 通用slab就是给kmalloc用的, 在/proc/slabinfo中，类似kmalloc-32的行，都是通用slab.
  * Link: https://argp.github.io/2012/01/03/linux-kernel-heap-exploitation/
  * Digest: As an example consider that a slab for the structure task_struct has 31 slots. The size of a task_struct is 1040 bytes, so assuming that a page is 4096 bytes (the default) then a task_struct slab is 8 pages long. Apart from the structure-specific slabs, like the one above for task_struct, there are also the so called general purpose slabs which are used to serve arbitrary-sized kmalloc() requests. These requests are adjusted by the allocator for alignment and assigned to a suitable slab.
