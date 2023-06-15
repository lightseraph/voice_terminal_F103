#include "syscall.h"

vs8 keypress_remain = 0;

void delay_nus(vu32 nus)
{
    if (SysTick_Config(SystemCoreClock / 1000000))
    {
        while (1)
            ;
    }
    time_delay = nus; // 读取定时时间
    while (time_delay)
        ;
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // 关闭计数器
    SysTick->VAL = 0X00;                       // 清空计数器
}

void delay_nms(vu32 nms)
{
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        while (1)
            ;
    }
    time_delay = nms; // 读取定时时间
    while (time_delay)
        ;
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // 关闭计数器
    SysTick->VAL = 0X00;                       // 清空计数器
}

void Flash_LED(LED_TYPE led, u16 interval, u8 count, LED_AFTER_FLASH cond)
{
    for (int i = 0; i < count * 2; i++)
    {
        if (led == LED_RED)
            HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
        else if (led == LED_GREEN)
            HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);

        delay_nms(interval);
    }
    if (cond != FOLLOW_PREVIOUS)
    {
        if (led == LED_RED)
            HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, (GPIO_PinState)cond);
        else if (led == LED_GREEN)
            HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, (GPIO_PinState)cond);
    }
}