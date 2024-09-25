#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

// 4K大小
#define PAGE_SIZE 0x1000
// 1M, 可用内存开始的位置, 因为1M的是固定保留的
#define MEMORY_BASE 0x100000

/**
 * 页目录和页表的结构体, 4字节
 * 
 * 假设有4G的内存, 每个页为4K, 那么就有4G/4K=1024x1024个页, 也就是2的20次方个页
 * 也就是其实使用20bit就可以表示所有的页, 也就是结构体中页索引为20位
 * 也就是如果仅仅只是用来标识页的索引, 其实20bit就足够了, 但是没有20bit的类型, 只能用32bit的类型
 * 多出来的12bit就能表示页的属性, 32bit的类型也就是下面的结构体
 * 
 * 1024x1024个页, 每个页需要用下面的结构体4B, 也就是需要4MB空间来存储下面的1024x1024个结构体
 * 这4MB空间需要4M/4K=1024个页来存储
 * 由于每个进程都有自己的页表, 那么如果每个进程都需要4MB来存储页表, 很明显是很耗费内存的
 * 而每个进程并不会使用所有的4G内存, 基本上都是很小的内存, 因此1024x1024个页不会全都使用
 * 
 * 因此我们可以采用某种方法来标识使用的页, 不使用的页不用放到进程的页表中
 * 我们看到4MB空间需要使用1024个页来存储, 这1024个页的每一个页都可以用下面的结构体来表示, 也就是4B
 * 那么1024个页就需要4K大小来表示, 也就是需要一个页就可以表示这1024个页的4MB空间, 也就能表示整个4G空间
 * 
 * 那么就是说我们做了两级页表, 第一级页表是页目录, 占4K, 共有1024个元素, 每个元素是4B, 表示二级页的属性, 那么二级页有1024个
 * 一级页表中的每个元素代表的一个二级页, 每个二级页有4K, 每个元素有4B, 那么一个二级页中有1024个元素, 每个二级页中的元素代表内存的一页
 * 那么一个二级页可以代表1024个页, 一共有1024个二级页, 也就可以代表1024x1024个页, 也就是4G
 * 那么可以推出来, 一个一级页表的元素, 可以代表1024个4K页, 也就是4MB
 * 
 * 那么对于一个进程来说, 有4MB可用空间, 他有一个页目录4K, 只需要将第一个元素的属性配置合适, 其他元素置为0
 * 4MB的二级页表中只需要有第一个就行了, 因为二级页的第一个4K页, 就能够代表1024x4K=4M的内存, 二级页的其他的就不需要了
 * 这样就可以用8K的代价表示4MB的可用空间, 而不是用4M的空间去表示4G的空间, 尽管这个进程只是使用了4M的空间
 */
typedef struct page_entry_t
{
    u8 present : 1;  // 在内存中
    u8 write : 1;    // 0 只读 1 可读可写
    u8 user : 1;     // 1 所有人 0 超级用户 DPL < 3
    u8 pwt : 1;      // page write through 1 直写模式，0 回写模式
    u8 pcd : 1;      // page cache disable 禁止该页缓冲
    u8 accessed : 1; // 被访问过，用于统计使用频率
    u8 dirty : 1;    // 脏页，表示该页缓冲被写过
    u8 pat : 1;      // page attribute table 页大小 4K/4M
    u8 global : 1;   // 全局，所有进程都用到了，该页不刷新缓冲
    u8 ignored : 3;  // 该安排的都安排了，送给操作系统吧
    u32 index : 20;  // 页索引
} _packed page_entry_t;

// 获取cr3寄存器
u32 get_cr3();
// 设置cr3寄存器
void set_cr3(u32 pde);

#endif