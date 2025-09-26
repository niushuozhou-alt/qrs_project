#include "stm32f10x.h"
#include "board.h"
#include "stdio.h"

int main(void)
{
    board_init();  // 初始化 SysTick，用于延时

    // 使能 GPIOA、GPIOB、GPIOC 时钟（APB2 总线）
    RCC->APB2ENR |= (1 << 2) | (1 << 3) | (1 << 4); 
	
    // 配置 PC13、PA8、PB14 为推挽输出模式（10MHz）
    GPIOC->CRH &= ~(0xF << (4 * (13 - 8))); 
    GPIOC->CRH |=  (0x1 << (4 * (13 - 8)));  

    GPIOA->CRH &= ~(0xF << (4 * (8 - 8))); 
    GPIOA->CRH |=  (0x1 << (4 * (8 - 8))); 

    GPIOB->CRH &= ~(0xF << (4 * (14 - 8))); 
    GPIOB->CRH |=  (0x1 << (4 * (14 - 8))); 

    // 初始设置：三个引脚输出高电平
    GPIOC->ODR |=  (1 << 13); 
    GPIOA->ODR |=  (1 << 8);   
    GPIOB->ODR |=  (1 << 14); 
	
    while (1)
    {
        // 熄灭 PC13，其余亮，延时1秒
        GPIOC->ODR &= ~(1 << 13); 
        GPIOA->ODR |=  (1 << 8);  
        GPIOB->ODR |=  (1 << 14); 
        delay_ms(1000);

        // 熄灭 PA8，其余亮，延时1秒
        GPIOC->ODR |=  (1 << 13); 
        GPIOA->ODR &= ~(1 << 8);   
        GPIOB->ODR |=  (1 << 14);  
        delay_ms(1000);

        // 熄灭 PB14，其余亮，延时1秒
        GPIOC->ODR |=  (1 << 13); 
        GPIOA->ODR |=  (1 << 8);  
        GPIOB->ODR &= ~(1 << 14); 
        delay_ms(1000);
    }
}
