#include "xinos/list.h"
#include "xinos/assert.h"

// 初始化链表
void list_init(list_t *list) {
    list->head.prev = NULL;
    list->tail.next = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
}

// 在anchor节点前面插入node节点
void list_insert_before(list_node_t *anchor, list_node_t *node) {
    node->prev = anchor->prev;
    node->next = anchor;
    anchor->prev->next = node;
    anchor->prev = node;
}

// 在anchor节点后面插入node节点
void list_insert_after(list_node_t *anchor, list_node_t *node) {
    node->next = anchor->next;
    node->prev = anchor;
    anchor->next->prev = node;
    anchor->next = node;
}

// 插入到头节点之后
void list_pushfront(list_t *list, list_node_t *node) {
    // 判断是否存在, 存在的话就是有问题的
    assert(!list_search(list, node));
    // 插入
    list_insert_after(&list->head, node);
}

// 移除头节点之后的节点
list_node_t *list_popfront(list_t *list) {
    // 判断是否为空
    assert(!list_empty(list));
    // 移除
    list_node_t *node = list->head.next;
    list_remove(node);

    return node;
}

// 插入到尾节点前
void list_pushback(list_t *list, list_node_t *node) {
    // 判断是否存在, 存在的话就是有问题的
    assert(!list_search(list, node));
    // 插入
    list_insert_before(&list->tail, node);
}

// 移除尾节点前的节点
list_node_t *list_popback(list_t *list) {
    // 判断是否为空
    assert(!list_empty(list));
    // 移除
    list_node_t *node = list->tail.prev;
    list_remove(node);

    return node;
}

// 查找链表中节点是否存在
bool list_search(list_t *list, list_node_t *node) {
    // 判断是否为空, 为空就是不存在
    if(list_empty(list)) return false;

    // 查找
    for(list_node_t *p = list->head.next; p != &list->tail; p = p->next) {
        if(p == node) return true;
    }
    return false;
}

// 从链表中删除节点
void list_remove(list_node_t *node) {
    // 判定前后不为空
    assert(node->prev != NULL);
    assert(node->next != NULL);

    // 开始删除
    list_node_t *p = node->prev;
    list_node_t *n = node->next;
    p->next = n;
    n->prev = p;
    node->next = node->prev = NULL;

    // todo 销毁内存 
}

// 判断链表是否为空
bool list_empty(list_t *list) {
    return (list->head.next == &list->tail);
}

// 获取链表长度
u32 list_size(list_t *list) {
    u32 size = 0;
    for(list_node_t *p = list->head.next; p != &list->tail; p = p->next) {
        size++;
    }
    return size;
}

#include "xinos/memory.h"
#include "xinos/debug.h"
#define LOGK(fmt, args...) DEBUGK(fmt, ##args)
void list_test()
{
    /**
     * 这里使用每一页作为一个节点, 那么在每页虚拟地址的开头位置就写入了prev和next节点的地址, 存到了内存中
     */
    u32 count = 3;
    list_t holder;
    list_t *list = &holder;
    list_init(list);
    list_node_t *node;
    while (count--)
    {
        node = (list_node_t *)alloc_kpage(1);
        list_pushfront(list, node);
    }
    while (!list_empty(list))
    {
        node = list_popfront(list);
        free_kpage((u32)node, 1);
    }
    count = 3;
    while (count--)
    {
        node = (list_node_t *)alloc_kpage(1);
        list_pushback(list, node);
    }
    LOGK("list size %d\n", list_size(list));
    while (!list_empty(list))
    {
        node = list_popback(list);
        free_kpage((u32)node, 1);
    }
    node = (list_node_t *)alloc_kpage(1);
    list_pushback(list, node);
    LOGK("search node 0x%p --> %d\n", node, list_search(list, node));
    LOGK("search node 0x%p --> %d\n", 0, list_search(list, 0));
    list_remove(node);
    free_kpage((u32)node, 1);
}