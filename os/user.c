#include "os.h"
#include "user_api.h"

#define DELAY 1000

lock_t lock;

/* 软件定时器测试 */
struct userdata {
	int counter;
	char *str;
};
struct userdata person = {0, "Jack"};
void timer_function(void *arg)
{
	if (NULL == arg)
		return;

	struct userdata *param = (struct userdata *)arg;

	param->counter++;
	printf("======> TIMEOUT: %s: %d\n", param->str, param->counter);
}

/* 用户任务0 */
void user_task1(void *param)
{
    printf("Task 1: Created & Started!\n");
    struct timer *t1 = timer_create(timer_function, &person, 3);
	if (NULL == t1) {
		printf("timer_create() failed!\n");
	}
	struct timer *t2 = timer_create(timer_function, &person, 5);
	if (NULL == t2) {
		printf("timer_create() failed!\n");
	}
	struct timer *t3 = timer_create(timer_function, &person, 7);
	if (NULL == t3) {
		printf("timer_create() failed!\n");
	}
    printf("Task 1: Running...\n");
    sleep(10);
    timer_delete(t1);
    timer_delete(t2);
    timer_delete(t3);
    printf("Task 1: Deleting...\n");
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
        task_delay(13);
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
        task_delay(17);
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