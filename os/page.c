#include "os.h"

/* 从mem.S中获取内存每段的位置 */
extern uint32_t TEXT_START;
extern uint32_t TEXT_END;
extern uint32_t RODATA_START;
extern uint32_t RODATA_END;
extern uint32_t DATA_START;
extern uint32_t DATA_END;
extern uint32_t BSS_START;
extern uint32_t BSS_END;
extern uint32_t HEAP_START;
extern uint32_t HEAP_END;
extern uint32_t HEAP_SIZE;

/*
 * 由于在heap的起始位置处需要存放每个页表的管理信息
 * 所以实际动态分配内存的起始地址为：HEAP_START + 所有管理信息的大小
 * 管理信息包括是否被使用,是否是一大段内存的终止位置等信息,每个4K页的管理信息用1字节就足够了
 * 128MB / 4KB = 32K,共32K个4K页,每个页管理信息占1字节,所以总共需要32KB存储管理信息
 * 又由于每个页为4KB,所以需要32KB / 4KB = 8,也就是8个4K页来存储管理信息
 * 所以实际动态分配内存的起始地址为：HEAP_START + 所有管理信息的大小 = HEAP_START + 8 * 4K
 */
static uint32_t _alloc_start = 0;
static uint32_t _alloc_end = 0;
static uint32_t _num_pages = 0;

#define PAGE_SIZE 4096

/* 用于4K对齐,4K是2的12次方,对齐方式见函数_align_page */
#define PAGE_ORDER 12

/* 页管理信息 */
#define PAGE_TAKEN (uint8_t)(1 << 0)
#define PAGE_LAST (uint8_t)(1 << 1)

/*
 * 页描述结构体
 * flags: 8 bits
 * bit 0: 是否被使用了PAGE_TAKEN
 * bit 1: 是否是一大段内存的最后一页
 */
struct Page
{
    uint8_t flags;
};

/* 清除页信息,也就是回收页 */
static inline void _clear(struct Page *page) {
    page->flags = 0;
}

/* 判断 */

/* page初始化 */
void page_init() {
    // 页总个数
    _num_pages = (HEAP_SIZE  / PAGE_SIZE) - 8;
    
}
