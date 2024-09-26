#include "xinos/memory.h"
#include "xinos/types.h"
#include "xinos/assert.h"
#include "xinos/debug.h"
#include "xinos/stdlib.h"
#include "xinos/string.h"

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

// 通过页的索引获取内存地址
#define PAGE(idx) ((u32)idx << 12)
// 判定是否是4K对齐
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

/**
 * ards结构体
 */
typedef struct ards_t {
    u64 base; // 内存基地址
    u64 size; // 内存长度
    u32 type; // 类型
} _packed ards_t;

// 可用内存的基地址, 一般是1M开始
static u32 memory_base = 0;
// 可用内存的大小
static u32 memory_size = 0;
// 页的总个数
static u32 total_pages = 0;
// 最大可用的页的个数
static u32 free_pages = 0;
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
    // 要求按页对齐, 就是看最后12位是否都为0
    // 因为我们设置的单位是4K, 所以内存大小要以4K为单位的
    // todo 这里是不是应该对memory_size向下对齐到4K比较好呢?
    assert((memory_size & 0xfff) == 0);

    // 可用页数
    free_pages = IDX(memory_size);
    // 总页数
    total_pages = IDX(MEMORY_BASE) + free_pages;

    // 页的个数
    LOGK("Total pages %d\n", total_pages);
    // 可用页的个数
    LOGK("Free pages %d\n", free_pages);
}

// 可分配物理内存的起始页
static u32 start_page = 0;
// 物理内存数组起始指针, 这里采用一个字节去表示一页内存被引用的数量, 最多255次
static u8 *memory_map;
// 物理内存数组所占用的页数
static u32 memory_map_pages;

/**
 * 物理内存数组设置
 */
void memory_map_init() {
    // 获取memory_map位置
    memory_map = (u8*)memory_base;
    /**
     * 查看物理内存数组的大小
     * 每一页占用一个字节, 一共有total_pages个页, 那么一共有total_pages个字节
     * 这些个字节占用多少页, 就是total_pages / PAGE_SIZE, 需要向上取整
     */
    memory_map_pages = div_round_up(total_pages, PAGE_SIZE);
    // 初始化物理内存数组
    memset((void*)memory_map, 0, memory_map_pages * PAGE_SIZE);
    LOGK("Memory map page count %d\n", memory_map_pages);

    // 更新可用内存页的个数
    free_pages -= memory_map_pages;
    // 更新可用内存页的位置, 前面1M不能用, 然后物理内存数组所在页不能用
    start_page = IDX(MEMORY_BASE) + memory_map_pages;

    // 设置start_page前面的内存已经使用了1次
    for(size_t i = 0; i < start_page; ++i) {
        memory_map[i] = 1;
    }

    LOGK("Total pages %d free pages %d\n", total_pages, free_pages);
}

/**
 * 分配一页物理内存
 */
static u32 get_page() {
    // 先看一下有没有自由内存页
    assert(free_pages > 0);
    // 循环查找
    for(size_t i = start_page; i < total_pages; ++i) {
        if(memory_map[i]) // 当前被占用
            continue;
        // 未被使用
        memory_map[i] = 1;
        free_pages--;
        // 返回地址, 而不是页的索引, 所以需要左移12位
        u32 page = ((u32)i) << 12;
        LOGK("GET page 0x%p\n", page);
        return page;
    }
    panic("Out of Memory!!!\n");
}

/**
 * 释放一页物理内存
 */
static void put_page(u32 addr) {
    // 确保过来释放的内存是4K对齐的
    ASSERT_PAGE(addr);
    // 获取页的索引
    u32 idx = IDX(addr);
    // 判断该页是在可用内存区域
    assert(idx >= start_page && idx < total_pages);
    // 保证是有引用的, 否则一个未被使用的内存是不能释放的
    assert(memory_map[idx] >= 1);

    // 释放内存页
    memory_map[idx]--;
    // 如果此时没有引用了, 就放到空闲中, 因此free_pages加1, 否则就是还有引用, 不能放到空闲中
    if(memory_map[idx] == 0) free_pages++;

    // 自由内存页个数应该大于等于0, 且由于前面1M和物理内存数组的存在, 肯定要小于total_pages
    assert(free_pages >= 0 && free_pages < total_pages);
    LOGK("PUT page 0x%p\n", addr);
}

// 获取cr3寄存器
u32 get_cr3() {
    asm volatile("movl %cr3, %eax\n");
}

// 设置cr3寄存器
void set_cr3(u32 pde) {
    ASSERT_PAGE(pde);
    /**
     * 扩展的内联汇编, 需要用两个%表示寄存器
     * a表示eax, "a"(pde)表示将pde的值放到eax中
     * 然后将eax的值赋给cr3
     */
    asm volatile("movl %%eax, %%cr3\n" :: "a"(pde));
}

/**
 * 将cr0寄存器最高位PE置为1, 启用分页
 */
static void enable_page()
{
    asm volatile(
        "movl %cr0, %eax\n"
        "orl $0x80000000, %eax\n"
        "movl %eax, %cr0\n");
}

/**
 * 初始化页表项
 * entry为页表入口的结构体
 * index表示索引
 */
static void entry_init(page_entry_t *entry, u32 index) {
    // 全都赋值为0
    *(u32 *)entry = 0;
    
    entry->present = 1;      // 在内存中
    entry->write = 1;        // 可读可写
    entry->user = 1;         // 所有人可用
    entry->index = index;    // 设置索引
}

// 内核页目录存放的位置
#define KERNEL_PAGE_DIR 0x200000
// 内核页表存放的位置, 因为页目录占用4K, 所以页表就多了0x1000
#define KERNEL_PAGE_ENTRY 0x201000

/**
 * 初始化内存映射
 */
void mapping_init() {
    /**
     * 我们先初始化大小为4MB的映射, 也就是只需要pde的第一个元素, pte的第一个4K
     */

    // 获取页目录表初始地址
    page_entry_t *pde = (page_entry_t *)KERNEL_PAGE_DIR;
    memset(pde, 0, PAGE_SIZE);
    // 设置第一个元素, 第一个元素的索引就是页表的第一个4K位置的索引
    entry_init(&pde[0], IDX(KERNEL_PAGE_ENTRY));

    // 设置pte的第一个4K, 每个元素占4B, 共1024个
    page_entry_t *pte = (page_entry_t *)KERNEL_PAGE_ENTRY;
    for(size_t i = 0; i < 1024; ++i) {
        /**
         * 这里要注意的是, pde中的索引是二级页表对应内存页所在的索引
         * 而pte由于直接指向的是内存页了, 所以索引是内存页的索引, 而不是下一级页表的索引
         * 像本程序中, 二级页表所在的位置是从KERNEL_PAGE_ENTRY开始的, 因此所在内存页不是从0开始的
         * 而pte指向的内存页就是内存, 就是从0开始的, 所以直接用i
         */
        entry_init(&pte[i], i);
        // 设置物理内存备用了
        memory_map[i] = 1;
    }

    // 设置cr3寄存器
    set_cr3((u32)pde);
    // 分页有效
    enable_page();
}

// 简单测试
void memory_test() {
    u32 pages[10];
    for (size_t i = 0; i < 10; i++)
    {
        pages[i] = get_page();
    }
    for (size_t i = 0; i < 10; i++)
    {
        put_page(pages[i]);
    }
}