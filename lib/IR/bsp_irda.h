#ifndef __BSP_IRDA_H_
#define __BSP_IRDA_H_

#include "main.h"

#if DEVICE_MODE_IRDA_SEND

#elif DEVICE_MODE_MOBILE

#define TIM_IC_PERIOD_US               (6000)
typedef struct
{
    /* 处于 FSM_RECVDATA 状态下等待的时间 */
    uint32_t fsm_recv_idle_time;

    /* 用于存储上升沿到下降沿持续的时间 */
    uint16_t ic_start;
    uint16_t ic_stop;
    uint16_t duration;

    /* 标识上升沿开始，下降沿结束 */
    uint8_t ic_polarity;

    /* 红外接收有限状态机
        0x00 = wait for leader code
        0x01 = recv data
    */
    #define IRDA_FMS_WAITLEADER     0x00
    #define IRDA_FMS_RECVDATA       0x01
    uint8_t IrDA_FMS;

    /* 存储红外 NEC 协议接收到的数据 */
    uint32_t IrDA_Data;
    uint8_t  IrDA_Data_Cnt;

    /* 数据重复的次数 */
    uint16_t IrDA_RepeatCNT;
}IrDA_t;

void Bsp_Irda_PostData(IrDA_t *pdata);
void Bsp_Irda_Calc(TIM_HandleTypeDef *htim , uint8_t channelx, IrDA_t *pdata);

#endif

#endif /* end of __BSP_IRDA_H_ */
