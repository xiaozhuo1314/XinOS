#include "os.h"

void lock_init(lock_t *lock)
{
    lock->locked = 0;
}

/* 在单hart抢占式系统中,死锁发生在中断任务切换时,所以粗暴的办法就是将中断关闭 */
void basic_lock()
{
    w_mstatus(r_mstatus() & ~MSTATUS_MIE);
}

/* 粗暴的释放锁 */
void basic_unlock()
{
    w_mstatus(r_mstatus() | MSTATUS_MIE);
}

/* 自旋锁,尝试获取锁状态并加锁 */
void lock_acquire(lock_t *lock)
{
    while(atomic_swap(lock));
}

/* 自旋锁,释放锁 */
void lock_free(lock_t *lock)
{
    lock->locked = 0;
}