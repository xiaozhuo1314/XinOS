#include "os.h"

#define DELAY 1000

lock_t lock;

/* 用户任务0 */
void user_task1(void *param)
{
    printf("Task 1: Created & Started!\n");
    int cnt = 0;
    while(1)
    {
        printf("Task 1: Running...\n");
        task_delay(DELAY);
        if((cnt++) > 1)
            break;
    }
    task_exit();
}

/* 用户任务1 */
void user_task2(void *param)
{
    printf("Task 2: Created & Started!\n");
    while(1)
    {
        printf("Task 2: Running...\n");
        lock_acquire(&lock);
        printf("Task 2 got lock\n");
        lock_free(&lock);
        task_delay(DELAY);
    }
    task_exit();
}

/* 用户任务2 */
void user_task3(void *param)
{
    printf("Task 3: Created & Started!\n");
    while(1)
    {
        printf("Task 3: Running...\n");
        lock_acquire(&lock);
        printf("Task 3 got lock\n");
        lock_free(&lock);
        task_delay(DELAY);
    }
    task_exit();
}

/* 创建所有用户任务函数 */
void user_init()
{
    task_create(user_task1, NULL, 100, 1);
    task_create(user_task2, NULL, 105, 1);
    task_create(user_task3, NULL, 110, 1);
}
