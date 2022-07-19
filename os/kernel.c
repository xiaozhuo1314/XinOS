extern void uart_init(void);
extern void uart_puts(char *p);
extern void uart_gets(void);
void start_kernel(void) {
    uart_init();
    uart_puts("hello world\n");
    uart_gets();
    while (1){}
}