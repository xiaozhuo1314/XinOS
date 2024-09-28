#include "xinos/bitmap.h"
#include "xinos/string.h"
#include "xinos/assert.h"

// 获取索引是在第几个字节
#define INDEX_BYTES(index) (index >> 3)
// 获取索引是在对应字节的第几个比特位
#define INDEX_BITS(index) (index & 7) 

// 构造位图
void bitmap_make(bitmap_t *map, char *bits, u32 length, u32 offset) {
    map->bits = bits;
    map->length = length;
    map->offset = offset;
}

// 初始化位图
void bitmap_init(bitmap_t *map, char *bits, u32 length, u32 offset) {
    memset((void*)map, 0, length);
    bitmap_make(map, bits, length, offset);
}

// 测试位图的某一位是否为 1, index是比特位
bool bitmap_test(bitmap_t *map, u32 index) {
    // 索引应该大于等于开始的偏移值
    assert(index >= map->offset);
    // 拿到相对索引, 因为offset在位图中是0, 所以需要相对值
    index -= map->offset;
    // 拿到索引是在第几个字节
    idx_t byte_idx = INDEX_BYTES(index);
    // 判断这个字节索引要小于length
    assert(byte_idx < map->length);
    // 拿到索引是在对应字节中的第几个比特位
    u8 bit_idx = INDEX_BITS(index);

    // return (map->bits[byte_idx] & (1 << bit_idx));
    return (((map->bits[byte_idx]) >> bit_idx) & 1);
}

// 设置位图某位的值, index是比特位
void bitmap_set(bitmap_t *map, u32 index, bool value) {
    // 索引应该大于等于开始的偏移值
    assert(index >= map->offset);
    // 拿到相对索引, 因为offset在位图中是0, 所以需要相对值
    index -= map->offset;
    // 拿到索引是在第几个字节
    idx_t byte_idx = INDEX_BYTES(index);
    // 判断这个字节索引要小于length
    assert(byte_idx < map->length);
    // 拿到索引是在对应字节中的第几个比特位
    u8 bit_idx = INDEX_BITS(index);

    if(value) map->bits[byte_idx] |= (1 << bit_idx);
    else map->bits[byte_idx] &= ~(1 << bit_idx);
}

// 从位图中得到连续的 count个为0的位
int bitmap_scan(bitmap_t *map, u32 count) {
    // 初始化开始位置
    int start = EOF;
    // 剩余多少位能检查
    u32 left = (map->length << 3);
    // 计数器
    int counter = 0;
    // 当前位
    u32 cur_bit = map->offset;
    while(left--) {
        if(bitmap_test(map, cur_bit)) counter = 0;  // 当前为1, 就只能重新找了
        else counter++;                             // 当前为0, 就需要加上

        // 判断是否满足要求了
        if(counter == count) {
            start = cur_bit - count + 1; // 由于cur_bit是从偏移开始的, 所以start中就已经加上偏移了
            break;
        }
        // 否则就继续
        cur_bit++;
    }

    // 如果没有找到
    if(start == EOF) {
        return EOF;
    }

    // 找到了就需要设置这些位为1
    while(cur_bit >= start) {
        bitmap_set(map, cur_bit--, true);
    }

    return start;
}

#include "xinos/debug.h"
#define LOGK(fmt, args...) DEBUGK(fmt, ##args)
#define LEN 2
u8 buf[LEN];  // 16个比特位的位图
bitmap_t map;
void bitmap_tests()
{
    bitmap_init(&map, buf, LEN, 0);
    for (size_t i = 0; i < 33; i++)
    {
        idx_t idx = bitmap_scan(&map, 1);
        if (idx == EOF)
        {
            LOGK("TEST FINISH\n");
            break;
        }
        LOGK("%d %d\n", i, idx);
    }
}