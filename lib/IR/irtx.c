#include "irtx.h"
#include "irda.h"
#include "tim.h"
#include "syscall.h"

void IR_PostData(uint8_t data)
{
    IR_HIGH(9000);
    IR_LOW(4500);
    IR_HIGH(560);

    Send_ByteData(LOCAL_ID[local_id]);
    Send_ByteData(~LOCAL_ID[local_id]);
    Send_ByteData(data);
    Send_ByteData(~data);
    HAL_GPIO_WritePin(IR_GPIO_Port, IR_Pin, RESET);
}

void IR_HIGH(uint16_t hold_time_us)
{
    HAL_GPIO_WritePin(IR_GPIO_Port, IR_Pin, RESET);
    HAL_TIM_Base_Start_IT(&htim1);
    IR_delay(hold_time_us);
    HAL_TIM_Base_Stop_IT(&htim1);
    HAL_GPIO_WritePin(IR_GPIO_Port, IR_Pin, RESET);
}

void IR_LOW(uint16_t hold_time_us)
{
    HAL_GPIO_WritePin(IR_GPIO_Port, IR_Pin, RESET);
    IR_delay(hold_time_us);
}

void Send_ByteData(uint8_t byte_data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if ((byte_data & (uint8_t)1 << i) == (uint8_t)1 << i)
        {
            IR_LOW(1680);
            IR_HIGH(560);
        }
        else
        {
            IR_LOW(560);
            IR_HIGH(560);
        }
    }
}