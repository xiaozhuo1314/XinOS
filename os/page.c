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

/* 页内block的大小 */
#define MALLOC_SIZE 4

/* 页内能分配成block的最大字节 */
#define ALLOCABLE_SIZE 3276

/* 页内管理block的管理信息的字节 */
#define MALLOC_TABLE_SIZE 820

/* 页管理信息 */
#define PAGE_TAKEN (uint8_t)(1 << 0)
#define PAGE_LAST (uint8_t)(1 << 1)
#define PAGE_FIRST (uint8_t)(1 << 2)
#define PAGE_MALLOC (uint8_t)(1 << 3) // 当前页是被malloc管理的
/* 与malloc和free有关的页/block管理信息 */
#define BLOCK_TAKEN (uint8_t)(1 << 0)
#define BLOCK_LAST (uint8_t)(1 << 1)
#define BLOCK_FIRST (uint8_t)(1 << 2)
#define PAGE_SOFT_FIRST (uint8_t)(1 << 4) // 表示前一页是被page_alloc分配的,不是malloc分配的

/* 1 << 5暂未使用, 1 << 6和1 << 7表示当前页中有多少block被使用 */

/*
 * 页描述结构体
 * flags: 8 bits
 *   bit 0: 是否被使用了
 *   bit 1: 是否是一大段内存的最后一页
 *   bit 2: 是否是一大段内存的第一页
 *   bit 3: 当前页是否是被malloc控制的
 */
struct Page
{
    uint8_t flags;
};

/*
 * block描述结构体
 * flags: 8 bits
 *   bit 0: 是否被使用了
 *   bit 1: 是否是内存的最后一个block
 *   bit 2: 是否是内存的第一个一个block
 *   bit 3: 暂未使用
 *   bit 4: 前一页是被page_alloc分配的,不是malloc分配的
 *   bit 5: 暂未使用
 *   bit 6: 表示当前页中有多少block被使用, 是10 bit位的倒数第二高位
 *   bit 7: 表示当前页中有多少block被使用, 是10 bit位的最高位
 *   这里的bit 6和bit 7只有第819 block才有意义
*/
struct Block
{
    uint8_t flags;
};


/* 清除页信息,也就是回收页 */
static inline void _clear(struct Page *page) {
    page->flags = 0;
}

/* 对齐函数 */
static inline uint32_t _align_page(uint32_t addr)
{
    uint32_t order = (1 << PAGE_ORDER) - 1;
    return ((addr + order) & (~order));
}

/* 判断页是否是空闲的 */
static inline int _is_free(struct Page *page)
{
    return (page->flags & PAGE_TAKEN) ? 0 : 1;
}

/* 设置flags */
static inline void _set_flags(struct Page *page, uint8_t flags)
{
    page->flags |= flags;
}

/* 判断是否是最后一个 */
static inline int _is_last(struct Page *page)
{
    return (page->flags & PAGE_LAST) ? 1 : 0;
}

/* 判断是否是malloc控制的页 */
static inline int _is_malloced(struct Page *page)
{
    return (page->flags & PAGE_MALLOC) ? 1 : 0;
}

/* 通过page获取页内存初始位置 */
static inline void *_get_mem_by_page(struct Page *page)
{
    return (void*)(_alloc_start + (page - (struct Page*)HEAP_START) * PAGE_SIZE);
}

/* 通过地址获取页内存的初始位置 */
static inline void *_get_mem_by_addr(void *p)
{
    uint32_t page_count = ((uint32_t)p - _alloc_start) / PAGE_SIZE;
    return (void*)(page_count * PAGE_SIZE + _alloc_start);
}

/* page初始化 */
void page_init() {
    // 页的总个数
    _num_pages = (HEAP_SIZE  / PAGE_SIZE) - 8;
    printf("HEAP_START = %x, HEAP_SIZE = %x, num of pages = %d\n", HEAP_START, HEAP_SIZE, _num_pages);
    
    // 初始化heap前面的页管理信息
    // 由于page是Page类型的指针,指向的为uint8_t的数值
    // 所以++page每次走过一个字节,而不是4K,所以虽然这里使用的是_num_pages,但是指的heap最前面的页管理信息
    // 因为后面的_num_pages个4K页的_num_pages个管理信息在前部
    struct Page *page = (struct Page*)HEAP_START;
    for(int i = 0; i < _num_pages; ++i)
    {
        _clear(page);
        ++page;
    }

    // 设置开始和结束开辟内存的地方,注意位置要4k对齐
    _alloc_start = _align_page(HEAP_START + 8 * PAGE_SIZE);
    _alloc_end = _alloc_start + _num_pages * PAGE_SIZE;

    printf("TEXT:   0x%x -> 0x%x\n", TEXT_START, TEXT_END);
	printf("RODATA: 0x%x -> 0x%x\n", RODATA_START, RODATA_END);
	printf("DATA:   0x%x -> 0x%x\n", DATA_START, DATA_END);
	printf("BSS:    0x%x -> 0x%x\n", BSS_START, BSS_END);
	printf("HEAP:   0x%x -> 0x%x\n", _alloc_start, _alloc_end);
}

/* 按照页个数分配内存 */
void *page_alloc(int npages)
{
    if(npages < 1)
    {
        return NULL;
    }
    int found = 0;
    // end_page指的是管理信息中的
    int end_page = _num_pages - npages;
    // page指向页管理信息的第0项
    // 后面的4K页是存放数据,前面的页管理信息才是控制4K页的属性,所以应该是对页管理信息进行修改
    // 而不是真正的4K页
    struct Page *page = (struct Page*)HEAP_START;
    struct Page *tmp = NULL;
    for(int i = 0; i <= end_page; ++i)
    {
        if(_is_free(page))
        {
            found = 1;
            tmp = page;
            for (int j = 1; j < npages; ++j)
            {
                if(!_is_free(++tmp))
                {
                    found = 0;
                    break;
                }
            }

            if(found)
            {
                tmp = page;
                _set_flags(page, PAGE_FIRST);
                for(int j = 0; j < npages; ++j)
                {
                    _set_flags(tmp++, PAGE_TAKEN);
                }
                // 最后一个还需要设置为last
                --tmp;
                _set_flags(tmp, PAGE_LAST);
                return (void*)(_alloc_start + i * PAGE_SIZE);
            }
        }
        ++page;
    }
    return NULL;
}

/* 按页释放内存 */
void page_free(void *p)
{
    if(!p || (uint32_t)p >= _alloc_end) return;

    struct Page *page = (struct Page *)HEAP_START + ((uint32_t)p - _alloc_start) / PAGE_SIZE;

    while(!_is_free(page))
    {
        if(_is_last(page))
        {
            _clear(page);
            break;
        }
        else
        {
            _clear(page++);
        }
    }
}

/* 
 * 通过页初始位置获取页中block的初始位置 
 * 参数为页的初始位置
 */
struct Block *_get_block_by_addr(void *p)
{
    return (struct Block*)(p + ALLOCABLE_SIZE - 1);
}

/*
 * block是否是free
 */
int _is_block_free(struct Block *block)
{
    return (block->flags & BLOCK_TAKEN) ? 0 : 1;
}

/* 设置block属性 */
void _set_block_flags(struct Block *block, uint8_t flags)
{
    block->flags |= flags;
}

/* 
 * 初始化malloc
 * p指的是页的初始地址 
 */
struct Block *_malloc_init(void *p)
{
    struct Block *block = _get_block_by_addr(p);
    struct Block *tmp = block;
    for (int i = 0; i < MALLOC_TABLE_SIZE; ++i, ++tmp)
    {
        tmp->flags = 0;
    }
    return block;
}

/* 增加一个页中block使用数目 */
static inline void _increase_block_usage_of_malloc(struct Block *block)
{
    // 页中block数目位的低地址block
    struct Block *low_block = block + MALLOC_TABLE_SIZE - 1;
    // 页中block数目位的高地址block
    struct Block *high_block = block + MALLOC_TABLE_SIZE - 2;
    // 加一
    if(low_block->flags == 255)
    {
        high_block->flags += (1 << 6);
    }
    /* 
     * 如果要是low_block->flags小于255,下面的语句是正确的
     * 如果要是low_block->flags等于255,正确的做法是low_block->flags加一后超过表示的最大数值而往前进一
     * 上面的语句high_block->flags += (1 << 6)相当于往前进一
     * 且low_block->flags++后low_block->flags为256,但是由于low_block->flags是8位的,所以此时会溢出变成0
     * 所以下面语句在low_block->flags等于255也是正确的
     */
    low_block->flags++; 
}

/* 尝试去分配内存 */
void *_try_malloc(void *p, size_t size)
{
    // 获取页的初始位置
    p = _get_mem_by_addr(p);
    // 获取页中block的初始位置
    struct Block *block, *block_start;
    block_start = block = _get_block_by_addr(p);
    // 需要多少个block
    int block_total = size / MALLOC_SIZE;
    if(size % MALLOC_SIZE) block_total += 1;
    int end_index = MALLOC_TABLE_SIZE - block_total;
    for(int i = 0; i < end_index; ++i, ++block) // 由于MALLOC_TABLE_SIZE比实际的block个数多1,所以这里不加等于
    {
        if(_is_block_free(block))
        {
            struct Block *tmp = block;
            int found = 1;
            for(int j = 1; j < block_total; ++j, ++tmp)
            {
                if(!_is_block_free(tmp))
                {
                    found = 0;
                    break;
                }
            }
            if(found)
            {
                tmp = block;
                _set_block_flags(block, BLOCK_FIRST);
                for(int j = 0; j < block_total; ++j, ++tmp)
                {
                    _set_block_flags(tmp, BLOCK_TAKEN);
                    // 页中block的使用数目加一
                    _increase_block_usage_of_malloc(block_start);
                }
                _set_block_flags(--tmp, BLOCK_LAST);
                return (void*)(p + i * MALLOC_SIZE);
            }
        }
    }
    return NULL;
}

/* 
 * 实现malloc
 * 在4K页内,每4字节作为一个block,4K的最后820字节作为当前页中block的管理信息
 * 前面的3276字节作为待分配的内存,3276字节共有819个block,4K共有1024个block
 * 所以将820字节的8位,加上819字节的最高2位,这样一共10位,正好能够覆盖所有的1024个block
 * 这样最后一个block的管理信息最多只能6位表示,由于管理信息具有普适性,那么所有的block的管理信息
 * 只需要最多6位即可,实际上在这个简易os上足够了
 */
void *malloc(size_t size)
{
    void *mem = NULL;
    /* 判断待分配内存的大小 */
    if(size <= ALLOCABLE_SIZE) // 按照malloc来分配
    {
        struct Page *page = (struct Page*)HEAP_START;
        for(int i = 0; i < _num_pages; ++i, ++page)
        {
            // 当前页是被malloc控制的,那么就尝试在这个里面分配内存
            if(_is_malloced(page))
            {
                
            }
            else if(_is_free(page)) // 如果不是被malloc控制但是空闲
            {
                // 设置页的属性
                _set_flags(page, PAGE_TAKEN | PAGE_LAST | PAGE_FIRST | PAGE_MALLOC);
                // 获取页内存初始位置
                mem = _get_mem_by_page(page);
                // 初始化malloc管理信息
                _malloc_init(mem);
                // 尝试去分配block内存,成功则返回,否则就进行下一循环
                if((mem = _try_malloc(mem, size)) != NULL) return mem;
            }
        }
    }
    else if(size > ALLOCABLE_SIZE && size <= PAGE_SIZE) // 直接按照一页来分配
    {

    }
    else // 前面的按照页,剩下的按照malloc分配
    {

    }
    return mem;
}

void page_test()
{
    void *p = page_alloc(1);
	printf("p = 0x%x\n", p);
	page_free(p);

	// void *p2 = page_alloc(7);
	// printf("p2 = 0x%x\n", p2);
	// page_free(p2);

	// void *p3 = page_alloc(4);
	// printf("p3 = 0x%x\n", p3);
}