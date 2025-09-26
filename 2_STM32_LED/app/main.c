#include "stm32f10x.h"
#include "board.h"
#include "bsp_uart.h"
#include "stdio.h"

int main(void)
{
	board_init();
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    
	
	GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_13;           
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;      
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;      
    GPIO_Init(GPIOC, &GPIO_InitStruct);                 
	
	
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_8;            
    GPIO_Init(GPIOA, &GPIO_InitStruct);                 

	
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_14;           
    GPIO_Init(GPIOB, &GPIO_InitStruct);                 

    GPIO_SetBits(GPIOC, GPIO_Pin_13);  
    GPIO_SetBits(GPIOA, GPIO_Pin_8);   
    GPIO_SetBits(GPIOB, GPIO_Pin_14);  

    while (1)	
    {
        
        GPIO_ResetBits(GPIOC, GPIO_Pin_13); 
        GPIO_SetBits(GPIOA, GPIO_Pin_8);    
        GPIO_SetBits(GPIOB, GPIO_Pin_14);   
        delay_1ms(1000);

        GPIO_SetBits(GPIOC, GPIO_Pin_13);   
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);  
        GPIO_SetBits(GPIOB, GPIO_Pin_14);   
        delay_1ms(1000);

        GPIO_SetBits(GPIOC, GPIO_Pin_13);   
        GPIO_SetBits(GPIOA, GPIO_Pin_8);    
        GPIO_ResetBits(GPIOB, GPIO_Pin_14); 
        delay_1ms(1000);
    }
	
	
}
