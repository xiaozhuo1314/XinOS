#include "os.h"

#define DELAY 1000

/* 用户任务0 */
void user_task1(void *param)
{
    printf("Task 0: Created & Started!\n");
    while(1)
    {
        printf("Task 0: Running...\n");
        task_delay(DELAY);
        back_os();
    }
}

/* 用户任务1 */
void user_task2(void *param)
{
    printf("Task 1: Created & Started!\n");
    while(1)
    {
        printf("Task 1: Running...\n");
        task_delay(DELAY);
        back_os();
    }
}

/* 用户任务2 */
void user_task3(void *param)
{
    printf("Task 2: Created & Started!\n");
    while(1)
    {
        printf("Task 2: Running...\n");
        task_delay(DELAY);
        back_os();
    }
}

/* 创建所有用户任务函数 */
void user_init()
{
    task_create(user_task1, NULL, 150);
    task_create(user_task2, NULL, 200);
    task_create(user_task3, NULL, 250);
}