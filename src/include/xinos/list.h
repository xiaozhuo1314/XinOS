/**
 * 数据结构-双向链表, 但是头尾不相接, 头尾是哨兵节点
 */

#ifndef LIST_H
#define LIST_H

#include "types.h"

// 找到结构体中的某一个元素, 相对于结构体起始地址的偏移量， 起始就是将结构体放到0内存处, 然后看对应元素的地址
#define element_offset(type, element) (u32)(&(((type *)0)->element))
// 通过结构体中的某一元素的地址, 找到结构体的起始地址
#define element_entry(type, element, ptr) (type *)((u32)ptr - element_offset(type, element))

// 链表节点
typedef struct list_node_t {
    struct list_node_t *prev;
    struct list_node_t *next;
}list_node_t;

// 链表
typedef struct list_t {
    list_node_t head;  // 头节点
    list_node_t tail;  // 尾节点
}list_t;

// 初始化链表
void list_init(list_t *list);

// 在anchor节点前面插入node节点
void list_insert_before(list_node_t *anchor, list_node_t *node);

// 在anchor节点后面插入node节点
void list_insert_after(list_node_t *anchor, list_node_t *node);

// 插入到头节点之后
void list_pushfront(list_t *list, list_node_t *node);

// 移除头节点之后的节点
list_node_t *list_popfront(list_t *list);

// 插入到尾节点前
void list_pushback(list_t *list, list_node_t *node);

// 移除尾节点前的节点
list_node_t *list_popback(list_t *list);

// 查找链表中节点是否存在
bool list_search(list_t *list, list_node_t *node);

// 从链表中删除节点
void list_remove(list_node_t *node);

// 判断链表是否为空
bool list_empty(list_t *list);

// 获取链表长度
u32 list_size(list_t *list);

#endif