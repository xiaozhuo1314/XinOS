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
/**
 * 获取页目录索引
 * 页目录索引是地址的最高10位, 0x3ff就是10个bit位为1
 */
#define DIDX(addr) (((u32)addr >> 22) & 0x3ff)
/**
 * 获取页表索引
 * 页表索引是中间的10位
 */
#define TIDX(addr) (((u32)addr >> 12) & 0x3ff)

// 通过页的索引获取内存地址
#define PAGE(idx) ((u32)idx << 12)
// 判定是否是4K对齐
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

/**
 * 内核页目录存放的位置, 本来0x1000是loader.bin的位置, 但是这个程序已经使用完毕了, 因此这个位置用来存放内核页目录
 * 页目录占用了4K
 */
#define KERNEL_PAGE_DIR 0x1000
/**
 * 页表存放的位置, 每个页表占4个字节
 * 由于页目录占用4K, 所以要从0x2000开始
 * 每个页表占用4K, 所以下一个页表是0x3000
 * 
 * 目前这里可以表示8M内存
 * 
 * 从0x2000开始到0x10000(不包含), 总共有14个地址可以使用, 那么就可以表示14x4M内存
 */
static u32 KERNEL_PAGE_TABLE[] = {
    0x2000,
    0x3000
};

/**
 * 内核内存的大小, 总共的
 * KERNEL_PAGE_TABLE中的每个页表可以表示4M内存, 有两个页表就是8M内存
 * 也就是4M * 页表个数 = 4M * (sizeof(KERNEL_PAGE_TABLE) / 4) = 1M * sizeof(KERNEL_PAGE_TABLE)
 * 也就是0x100000 * sizeof(KERNEL_PAGE_TABLE)
 * 
 */
#define KERNEL_MEMORY_SIZE (0x100000 * sizeof(KERNEL_PAGE_TABLE))

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
 * 物理内存初始化
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
    // 看一下检查出来的大小是否小于规定的大小, 如果小于的话, 说明内存条内存不够
    if(memory_size < KERNEL_MEMORY_SIZE) {
        panic("System memory is %dM too small, at least %dM needed\n",
              memory_size / 0x100000, KERNEL_MEMORY_SIZE / 0x100000);
    }

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
    // 获取memory_map位置, 此时是1M的位置
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
static _inline void enable_page()
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

    // 内存页的索引
    idx_t index = 0;
    /**
     * 遍历每个页表的位置, 然后页表中的每4B表示一个页
     */
    for(idx_t didx = 0; didx < sizeof(KERNEL_PAGE_TABLE) / 4; ++didx) {
        /**
         * 1. 向页目录中设置
         */
        // 1.1 清空页表的页
        page_entry_t *pte = (page_entry_t*)KERNEL_PAGE_TABLE[didx];
        memset((void*)pte, 0, PAGE_SIZE);
        // 1.2 设置页目录中的页表, 例如didx为0的时候, 就将2设置到了page_entry_t的index上
        // 也就是说didx代表的页表的位置是内存页的2号索引的位置, 也就是0x2000的位置
        entry_init(&pde[didx], IDX((u32)pte));
        /**
         * 2. 设置页表
         */
        for(idx_t tidx = 0; tidx < 1024; ++tidx, ++index) {
            // 第0页不做映射, 用于空指针访问和缺页异常等
            // 这里不能用tidx==0判断, 因此每一个didx循环都会有tidx为0的时候, 而我们只需要第0页不映射
            if(index == 0) continue;
            // 映射
            entry_init(&pte[tidx], index);
            // 设置物理内存该页被占用?
            memory_map[index] = 1;
        }
    }

    /**
     * 将最后一个页目录项指向页目录自己
     * 这样的话, 在寻址的时候, 找到最后一个页目录项, cpu就找了页表, 恰好这个页表就是页目录
     * 然后通过这个页表上面的page_entry_t就可以找到各个页, 这些页恰好就是页表
     * 因此就可以修改这些页表所在的页了
     * 而且由于页目录指代的页表的最后一个页表项指向的还是自己, 因此此时也就找到了页目录
     * 
     * 因此就可以通过虚拟内存来进行修改页目录和页表了
     * 为什么不能通过cr3寄存器来修改页目录呢?
     * 因为我们通过cr3获取到的地址, 读写这个地址的时候, 由于已经开启了分页机制
     * cpu会将这个地址拿出最高10位找页目录中的索引, 然后中间10位找页表, 然后再找页, 很明显此时不是页目录所在的页了
     * 
     * 因此我们采用上面的机制, 就可以通过读写0xfffff000-0xffffffff来修改页目录了
     * 那么页表修改的话, 最高10位是1, 就能拿到最后一个页目录项, 此时cpu认为页目录就是一个页表
     * 然后修改中间10位就可以得到不同的页的位置, 也就是页表所在页的位置, 就可以修改了
     */
    entry_init(&pde[1023], IDX(KERNEL_PAGE_DIR));

    // 设置cr3寄存器
    set_cr3((u32)pde);
    // 分页有效
    enable_page();
}

/**
 * 获取页目录的虚拟内存位置, 已经开启分页了
 */
static page_entry_t *get_pde() {
    return (page_entry_t *)(0xfffff000);
}

/**
 * 获取某个虚拟地址的页表的虚拟内存位置
 */
static page_entry_t *get_pte(u32 vaddr) {
    // 下面这三行是获取的页表的真实地址
    // page_entry_t* pde = get_pde();
    // idx_t didx = DIDX(vaddr);
    // return (page_entry_t*)(pde[didx].index << 12);

    /**
     * 0xffc00000表示最高10位为1
     * DIDX(vaddr) << 12表示将页目录索引放到了中间10位
     * 
     * 这样返回的虚拟地址, CPU首先拿到最高10位找到最后一个页目录项, 也就是指向了页目录本身
     * 这样CPU就将页目录当成了页表
     * 然后用中间10位, 也就是原来的页目录索引找到了页表在页目录中的项, 通过index找到了页表的真实位置
     */
    return (page_entry_t*)(0xffc00000 | (DIDX(vaddr) << 12));
}

// 刷新虚拟地址 vaddr 的 块表 TLB
static void flush_tlb(u32 vaddr) {
    asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
}

// 简单测试
void memory_test() {
    BMB;
    // 将 20 M 0x1400000 内存映射到 64M 0x4000000 的位置
    // 我们还需要一个页表，0x900000
    u32 vaddr = 0x4000000; // 线性地址几乎可以是任意的
    u32 paddr = 0x1400000; // 物理地址必须要确定存在
    u32 table = 0x900000;  // 页表也必须是物理地址

    // 将页表地址的索引放到页目录中, 要放的是物理地址的索引, 这样下面的get_pte获得的虚拟地址才会跟这个物理地址对应, 才能添加页表项
    page_entry_t *pde = get_pde();
    page_entry_t *dentry = &pde[DIDX(vaddr)];
    entry_init(dentry, IDX(table));
    // 获取刚才放到页目录中的页表的虚拟地址
    page_entry_t *pte = get_pte(vaddr);
    // 获取页表, cpu会把获取的页表的虚拟地址根据页目录转换成物理地址
    page_entry_t *tentry = &pte[TIDX(vaddr)];
    // 添加页表项的索引等属性, 这里需要真实物理地址
    entry_init(tentry, IDX(paddr));
    BMB;
    char *ptr = (char *)(0x4000000);
    ptr[0] = 'a';
    BMB;
    // 由于前面我们对tentry对应的页表进行了赋值操作, 这个页表已经被拷贝到了块表中, 所以这里修改后需要刷新
    entry_init(tentry, IDX(0x1500000));
    flush_tlb(vaddr);
    BMB;
    ptr[2] = 'b';
    BMB;
}