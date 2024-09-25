#include "xinos/memory.h"
#include "xinos/types.h"
#include "xinos/assert.h"
#include "xinos/debug.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

/**
 * ards结构体中的type是标识内存地址是否可用
 * 1表示可用, 2表示不可用
 */
#define ZONE_VALID 1    // 可用
#define ZONE_RESERVED 2 // 不可用

/**
 * 获取内存地址所在页的索引
 * 4K就是2的12次方
 */
#define IDX(addr) ((u32)addr >> 12)

/**
 * ards结构体
 */
typedef struct ards_t {
    u64 base; // 内存基地址
    u64 size; // 内存长度
    u32 type; // 类型
} _packed ards_t;

// 可用内存的基地址, 一般是1M开始
u32 memory_base = 0;
// 可用内存的大小
u32 memory_size = 0;
// 最大页的索引, 包含最开始的1M
u32 total_pages = 0;
// 最大可用页的索引
u32 free_pages = 0;
// 已经使用的内存页数
#define used_pages (total_pages - free_pages)

/**
 * 内存初始化
 */
void memory_init(u32 magic, u32 addr) {
    if(magic != KERNEL_MAGIC) {
        panic("magic code is not equal to %d, it is %d\n", KERNEL_MAGIC, magic);
    }
    u32 count = *(u32*)addr;
    /**
     * 由于我们在loader.asm中将结构体起始地址ards_buffer放到了ards_count下面
     * 而且ards_count占用4个字节, 因此我们只需要将addr向后移动4个字节即可
     * 
     * 这里我们要注意的是
     * addr是u32的一个数值, 虽然是内存地址的数值, 但毕竟是个数值而不是指针
     * 所以addr+1并不是往后增加4个字节, 而是数值加1
     * 所以这里应该是addr+4, 也就是数值加4, 跳过4个字节
     * 如果addr是个指针的话, 那么+1就是往后增加了一个u32大小, 也就是4个字节
     * 
     * 所以针对指针的++是向后跳过一个类型的大小
     * 对数值的++就是针对数值加1
     */
    ards_t *ptr = (ards_t*)(addr + 4);


    // 遍历所有结构体
    for(size_t i = 0; i < count; ++i, ++ptr) {
        LOGK("Memory base 0x%p size 0x%p type %d\n", (u32)ptr->base, (u32)ptr->size, (u32)ptr->type);
        // 我们这里使用最大的那一块内存, 一般是1M开始的那个为最大内存
        if(ptr->type == ZONE_VALID && ptr->size > memory_size) {
            memory_base = (u32)(ptr->base);
            memory_size = (u32)(ptr->size);
        }
    }

    LOGK("ARDS count %d\n", count);
    LOGK("Memory base 0x%p\n", (u32)memory_base);
    LOGK("Memory size 0x%p\n", (u32)memory_size);

    // 判断内存开始的位置是否是1M
    assert(memory_base == MEMORY_BASE);
    // 要求按页对其, 就是看最后12位是否都为0
    assert((memory_size & 0xfff) == 0);

    // 可用页数, 起始就是页的最大索引
    free_pages = IDX(memory_size);
    total_pages = IDX(MEMORY_BASE) + free_pages;

    // 页的个数就是最大索引+1
    LOGK("Total pages %d\n", total_pages + 1);
    // 可用页的个数就是最大可用页索引+1
    LOGK("Free pages %d\n", free_pages + 1);
}