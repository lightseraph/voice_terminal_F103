#include "bsp_irda.h"
#include "tim.h"
// STM32 stdPri lib
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"

#if DEVICE_MODE_IRDA_SEND

#elif DEVICE_MODE_MOBILE

void Bsp_Irda_PostData(IrDA_t *pdata)
{
    if(pdata->IrDA_Data > 0)
        
    pdata->IrDA_FMS           = IRDA_FMS_WAITLEADER;
    pdata->IrDA_Data          = 0;
    pdata->IrDA_Data_Cnt      = 0;
    pdata->IrDA_RepeatCNT     = 0;
    pdata->fsm_recv_idle_time = 0;
}

void Bsp_Irda_Calc(TIM_HandleTypeDef *htim , uint8_t channelx, IrDA_t *pdata)
{   
    TIM_IC_InitTypeDef sConfigIC = {0 , TIM_ICSELECTION_DIRECTTI , TIM_ICPSC_DIV1 , 0};

    if(pdata->ic_polarity == 0)
    {
        pdata->ic_polarity = 1;

        switch(channelx) //记录捕获时刻TIMx_CNT的值
        {
            case 1: pdata->ic_start = __HAL_TIM_GET_COUNTER(htim); 
                    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
                    HAL_TIM_IC_ConfigChannel(&htim, &sConfigIC, TIM_CHANNEL_1);
                    break;

            case 2: pdata->ic_start = __HAL_TIM_GET_COUNTER(htim); 
                    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
                    HAL_TIM_IC_ConfigChannel(&htim, &sConfigIC, TIM_CHANNEL_2);
                    break;

            default: break;
        }			
    }
    else
    {
        pdata->ic_polarity = 0;

        switch(channelx)//记录捕获时刻TIMx_CNT的值
        {
            case 1: pdata->ic_start = __HAL_TIM_GET_COUNTER(htim); 
                    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
                    HAL_TIM_IC_ConfigChannel(&htim, &sConfigIC, TIM_CHANNEL_1);
                    break;

            case 2: pdata->ic_start = __HAL_TIM_GET_COUNTER(htim); 
                    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
                    HAL_TIM_IC_ConfigChannel(&htim, &sConfigIC, TIM_CHANNEL_2);
                    break;

            default: break;
        }

        //calc the duration time
        if(pdata->ic_stop > pdata->ic_start)
            pdata->duration = pdata->ic_stop - pdata->ic_start;
        else
            pdata->duration = (TIM_IC_PERIOD_US - pdata->ic_start) + pdata->ic_stop;

        if(pdata->IrDA_FMS == IRDA_FMS_RECVDATA)
        {
            // 注意：此处每个节拍耗时 10us
            // if(pdata->duration > 284 && pdata->duration < 414)
            if(pdata->duration > 30 && pdata->duration < 70)
            {
                pdata->IrDA_Data <<= 1;
                pdata->IrDA_Data |= 0;

                // clear time_snap
                pdata->fsm_recv_idle_time = 0;
                pdata->IrDA_Data_Cnt++;
            }
						
            // else if(pdata->duration > 800 && pdata->duration < 1200)
            else if(pdata->duration > 140 && pdata->duration < 180)
            {
                pdata->IrDA_Data <<= 1;
                pdata->IrDA_Data |= 1;

                // clear time_snap
                pdata->fsm_recv_idle_time = 0;
                pdata->IrDA_Data_Cnt++;
            }
            // else if(pdata->duration > 200 && pdata->duration < 240)
            // {
            //     pdata->IrDA_RepeatCNT++;
            // }

            // exit the recv status
            if(pdata->IrDA_Data_Cnt >= 32)
                bsp_irda_post_data(pdata);
        }
        else
        {
            // received leader code, turn into recv status.
            // if(pdata->duration > 2000 && pdata->duration < 3200)
            if(pdata->duration > 400 && pdata->duration < 500)
            {
                pdata->IrDA_FMS       = IRDA_FMS_RECVDATA;
                pdata->IrDA_Data      = 0;
            }
        }
    }
}

#endif
