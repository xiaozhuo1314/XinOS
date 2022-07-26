#include "os.h"

#define DELAY 1000

/* 用户任务0 */
void user_task0()
{
    printf("Task 0: Created & Started!\n");
    while(1)
    {
        printf("Task 0: Running...\n");
        task_delay(DELAY);
        task_yield();
    }
}

/* 用户任务1 */
void user_task1()
{
    printf("Task 1: Created & Started!\n");
    while(1)
    {
        printf("Task 1: Running...\n");
        task_delay(DELAY);
        task_yield();
    }
}

/* 用户任务2 */
void user_task2()
{
    printf("Task 2: Created & Started!\n");
    while(1)
    {
        printf("Task 2: Running...\n");
        task_delay(DELAY);
        task_yield();
    }
}

/* 创建所有任务函数 */
void os_main()
{
    task_create(user_task0);
    task_create(user_task1);
    task_create(user_task2);
}