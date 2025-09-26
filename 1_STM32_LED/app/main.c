#include "stm32f10x.h"
#include "board.h"
#include "stdio.h"

int main(void)
{
    board_init();  // ��ʼ�� SysTick��������ʱ

    // ʹ�� GPIOA��GPIOB��GPIOC ʱ�ӣ�APB2 ���ߣ�
    RCC->APB2ENR |= (1 << 2) | (1 << 3) | (1 << 4); 
	
    // ���� PC13��PA8��PB14 Ϊ�������ģʽ��10MHz��
    GPIOC->CRH &= ~(0xF << (4 * (13 - 8))); 
    GPIOC->CRH |=  (0x1 << (4 * (13 - 8)));  

    GPIOA->CRH &= ~(0xF << (4 * (8 - 8))); 
    GPIOA->CRH |=  (0x1 << (4 * (8 - 8))); 

    GPIOB->CRH &= ~(0xF << (4 * (14 - 8))); 
    GPIOB->CRH |=  (0x1 << (4 * (14 - 8))); 

    // ��ʼ���ã�������������ߵ�ƽ
    GPIOC->ODR |=  (1 << 13); 
    GPIOA->ODR |=  (1 << 8);   
    GPIOB->ODR |=  (1 << 14); 
	
    while (1)
    {
        // Ϩ�� PC13������������ʱ1��
        GPIOC->ODR &= ~(1 << 13); 
        GPIOA->ODR |=  (1 << 8);  
        GPIOB->ODR |=  (1 << 14); 
        delay_ms(1000);

        // Ϩ�� PA8������������ʱ1��
        GPIOC->ODR |=  (1 << 13); 
        GPIOA->ODR &= ~(1 << 8);   
        GPIOB->ODR |=  (1 << 14);  
        delay_ms(1000);

        // Ϩ�� PB14������������ʱ1��
        GPIOC->ODR |=  (1 << 13); 
        GPIOA->ODR |=  (1 << 8);  
        GPIOB->ODR &= ~(1 << 14); 
        delay_ms(1000);
    }
}
