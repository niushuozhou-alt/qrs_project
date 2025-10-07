#include "stm32f10x.h"
#include "board.h"
#include "bsp_uart.h"
#include "stdio.h"
#include <stdio.h>
#include <stdlib.h>

// 全局变量
int global_var = 100;
int global_uninit;
const int global_const = 200;
static int static_global = 300;

void test_function(void) {
    static int static_local = 400;
    int local_var = 500;
    int *heap_var = (int *)malloc(sizeof(int));
    *heap_var = 600;

    printf("=== Inside test_function ===\r\n");
    printf("static_local (static local) : 0x%08X\r\n", (uint32_t)&static_local);
    printf("local_var (stack)           : 0x%08X\r\n", (uint32_t)&local_var);
    printf("heap_var (heap)             : 0x%08X\r\n", (uint32_t)heap_var);
    printf("*heap_var = %d\r\n", *heap_var);

    free(heap_var);
}



int main(void)
{
	board_init();

    uart1_init(115200);
	
	
	printf("\r\n=== STM32F103 Memory Layout Demo ===\r\n");
    printf("global_var (initialized)    : 0x%08X\r\n", (uint32_t)&global_var);
    printf("global_uninit (uninit)      : 0x%08X\r\n", (uint32_t)&global_uninit);
    printf("global_const (const)        : 0x%08X\r\n", (uint32_t)&global_const);
    printf("static_global (static)      : 0x%08X\r\n", (uint32_t)&static_global);

    test_function();
	
	
    while (1)	
    {
        

    }
	
	
}
