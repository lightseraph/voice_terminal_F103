
#include "bsp_irda.h"


#include "includes.h"

// STM32 stdPri lib
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"


extern OS_EVENT* os_queue_irda;

// 节拍 10us
// 周期 10 ms
#define TIM_IC_PERIOD_US               (6000)



struct IrDA_t
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
};


#if (CFG_DEVICE_MODE == DEVICE_MODE_IRDA_SEND)
void bsp_irda_send_config(void)
{
    GPIO_InitTypeDef gpio_initstructure;
    TIM_TimeBaseInitTypeDef tim_timebasestructure;
    TIM_OCInitTypeDef tim_ocinitstructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    gpio_initstructure.GPIO_Pin   = GPIO_Pin_8;
    gpio_initstructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio_initstructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio_initstructure);

    // 节拍周期 1us
    tim_timebasestructure.TIM_Period            = (26 - 1);
    tim_timebasestructure.TIM_Prescaler         = (72 - 1);
    tim_timebasestructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    tim_timebasestructure.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_timebasestructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &tim_timebasestructure);

    tim_ocinitstructure.TIM_OCMode       = TIM_OCMode_PWM1;
    tim_ocinitstructure.TIM_OutputState  = TIM_OutputState_Enable;
    tim_ocinitstructure.TIM_OutputNState = TIM_OutputNState_Disable;
    tim_ocinitstructure.TIM_Pulse        = 13;
    tim_ocinitstructure.TIM_OCPolarity   = TIM_OCPolarity_High;
    tim_ocinitstructure.TIM_OCNPolarity  = TIM_OCNPolarity_High;
    tim_ocinitstructure.TIM_OCIdleState  = TIM_OCIdleState_Reset;
    tim_ocinitstructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OC3Init(TIM4, &tim_ocinitstructure);
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
}

void bsp_irda_send_enable(void)
{
    TIM_SetCounter(TIM4, 0);

    TIM_CCxCmd(TIM4, TIM_Channel_3, TIM_CCx_Enable);
    TIM_Cmd(TIM4, ENABLE);
}

void bsp_irda_send_disable(void)
{
    TIM_Cmd(TIM4, DISABLE);
    TIM_CCxCmd(TIM4, TIM_Channel_3, TIM_CCx_Disable);
}

#endif

#if (CFG_DEVICE_MODE == DEVICE_MODE_MOBILE)
void bsp_irda_recv_config(void)
{
    GPIO_InitTypeDef gpio_initstructure;
    NVIC_InitTypeDef nvic_initstructure;
    TIM_TimeBaseInitTypeDef tim_timebasestructure;
    TIM_ICInitTypeDef tim_icinitstructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB| RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    gpio_initstructure.GPIO_Pin   = GPIO_Pin_3 | GPIO_Pin_10 | GPIO_Pin_11;
    gpio_initstructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    gpio_initstructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio_initstructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_10 | GPIO_Pin_11);
	
	  gpio_initstructure.GPIO_Pin   = GPIO_Pin_15;
    gpio_initstructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    gpio_initstructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_initstructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_15);
	
	  GPIO_PinRemapConfig(GPIO_FullRemap_TIM2,ENABLE);
	  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE); 

    // 节拍 10us
    // 周期 10 ms
    tim_timebasestructure.TIM_Period            = (TIM_IC_PERIOD_US - 1);
    tim_timebasestructure.TIM_Prescaler         = (720 - 1);
    tim_timebasestructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    tim_timebasestructure.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_timebasestructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &tim_timebasestructure);

    /* tim2, channel 1 */
    tim_icinitstructure.TIM_Channel     = TIM_Channel_1;
    tim_icinitstructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;
    tim_icinitstructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    tim_icinitstructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    tim_icinitstructure.TIM_ICFilter    = 0;
    TIM_ICInit(TIM2, &tim_icinitstructure);

    /* tim2, channel 2 */
    tim_icinitstructure.TIM_Channel     = TIM_Channel_2;
    tim_icinitstructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;
    tim_icinitstructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    tim_icinitstructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    tim_icinitstructure.TIM_ICFilter    = 0;
    TIM_ICInit(TIM2, &tim_icinitstructure);

    /* tim2, channel 3 */
    tim_icinitstructure.TIM_Channel     = TIM_Channel_3;
    tim_icinitstructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;
    tim_icinitstructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    tim_icinitstructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    tim_icinitstructure.TIM_ICFilter    = 0;
    TIM_ICInit(TIM2, &tim_icinitstructure);

    /* tim2, channel 4 */
    tim_icinitstructure.TIM_Channel     = TIM_Channel_4;
    tim_icinitstructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;
    tim_icinitstructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    tim_icinitstructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    tim_icinitstructure.TIM_ICFilter    = 0;
    TIM_ICInit(TIM2, &tim_icinitstructure);

    nvic_initstructure.NVIC_IRQChannel                   = TIM2_IRQn;
    nvic_initstructure.NVIC_IRQChannelPreemptionPriority = 1;
    nvic_initstructure.NVIC_IRQChannelSubPriority        = 3;
    nvic_initstructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic_initstructure);

    TIM_ClearFlag(TIM2, TIM_IT_Update | TIM_IT_CC1 |
                                        TIM_IT_CC2 |
                                        TIM_IT_CC3 |
                                        TIM_IT_CC4);
    TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_CC1 |
                                       TIM_IT_CC2 |
                                       TIM_IT_CC3 |
                                       TIM_IT_CC4, ENABLE);

    TIM_Cmd(TIM2, ENABLE);
}

void bsp_tim3_irda_recv_config(void)
{
    GPIO_InitTypeDef gpio_initstructure;
    NVIC_InitTypeDef nvic_initstructure;
    TIM_TimeBaseInitTypeDef tim_timebasestructure;
    TIM_ICInitTypeDef tim_icinitstructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB| RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    gpio_initstructure.GPIO_Pin   = GPIO_Pin_4 | GPIO_Pin_5;
    gpio_initstructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    gpio_initstructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio_initstructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_4 | GPIO_Pin_5);
	
	
	  GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3,ENABLE);
	  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE); 

    // 节拍 10us
    // 周期 10 ms
    tim_timebasestructure.TIM_Period            = (TIM_IC_PERIOD_US - 1);
    tim_timebasestructure.TIM_Prescaler         = (720 - 1);
    tim_timebasestructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    tim_timebasestructure.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_timebasestructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &tim_timebasestructure);

    /* tim3, channel 1 */
    tim_icinitstructure.TIM_Channel     = TIM_Channel_1;
    tim_icinitstructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;
    tim_icinitstructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    tim_icinitstructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    tim_icinitstructure.TIM_ICFilter    = 0;
    TIM_ICInit(TIM3, &tim_icinitstructure);

    /* tim3, channel 2 */
    tim_icinitstructure.TIM_Channel     = TIM_Channel_2;
    tim_icinitstructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;
    tim_icinitstructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    tim_icinitstructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    tim_icinitstructure.TIM_ICFilter    = 0;
    TIM_ICInit(TIM3, &tim_icinitstructure);

    nvic_initstructure.NVIC_IRQChannel                   = TIM3_IRQn;
    nvic_initstructure.NVIC_IRQChannelPreemptionPriority = 1;
    nvic_initstructure.NVIC_IRQChannelSubPriority        = 4;
    nvic_initstructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic_initstructure);

    TIM_ClearFlag(TIM3, TIM_IT_Update | TIM_IT_CC1 |
                                        TIM_IT_CC2 //|
                                       // TIM_IT_CC3 |
                                       // TIM_IT_CC4
																			 );
    TIM_ITConfig(TIM3, TIM_IT_Update | TIM_IT_CC1 |
                                       TIM_IT_CC2 //|
                                      // TIM_IT_CC3 |
                                      // TIM_IT_CC4, 
																			,ENABLE);

    TIM_Cmd(TIM3, ENABLE);
}


void bsp_irda_post_data(struct IrDA_t *pdata)
{
    if(pdata->IrDA_Data > 0)
        OSQPost(os_queue_irda, (void *)pdata->IrDA_Data);

    pdata->IrDA_FMS           = IRDA_FMS_WAITLEADER;
    pdata->IrDA_Data          = 0;
    pdata->IrDA_Data_Cnt      = 0;
    pdata->IrDA_RepeatCNT     = 0;
    pdata->fsm_recv_idle_time = 0;
}

void bsp_irda_channelx_calc(uint8_t channelx, struct IrDA_t *pdata)
{   
    if(pdata->ic_polarity == 0)
    {
        pdata->ic_polarity = 1;

        switch(channelx) //记录捕获时刻TIMx_CNT的值
        {
            case 1: pdata->ic_start = TIM_GetCapture1(TIM2); TIM_OC1PolarityConfig(TIM2, TIM_ICPolarity_Falling); break;
            case 2: pdata->ic_start = TIM_GetCapture2(TIM2); TIM_OC2PolarityConfig(TIM2, TIM_ICPolarity_Falling); break;
            case 3: pdata->ic_start = TIM_GetCapture3(TIM2); TIM_OC3PolarityConfig(TIM2, TIM_ICPolarity_Falling); break;
            case 4: pdata->ic_start = TIM_GetCapture4(TIM2); TIM_OC4PolarityConfig(TIM2, TIM_ICPolarity_Falling); break;
            default: break;
        }
				
    }
    else
    {
        pdata->ic_polarity = 0;

        switch(channelx)//记录捕获时刻TIMx_CNT的值
        {
            case 1: pdata->ic_stop = TIM_GetCapture1(TIM2); TIM_OC1PolarityConfig(TIM2, TIM_ICPolarity_Rising); break;
            case 2: pdata->ic_stop = TIM_GetCapture2(TIM2); TIM_OC2PolarityConfig(TIM2, TIM_ICPolarity_Rising); break;
            case 3: pdata->ic_stop = TIM_GetCapture3(TIM2); TIM_OC3PolarityConfig(TIM2, TIM_ICPolarity_Rising); break;
            case 4: pdata->ic_stop = TIM_GetCapture4(TIM2); TIM_OC4PolarityConfig(TIM2, TIM_ICPolarity_Rising); break;
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

void bsp_irda_isr(void)
{
    static struct IrDA_t irda_data[4];
    uint8_t idx = 0;

    // 节拍 10us
    // 周期 10 ms
	  
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

        for(idx = 0; idx < 4; ++idx)
        {
            if(irda_data[idx].IrDA_FMS == IRDA_FMS_WAITLEADER) continue;

            irda_data[idx].fsm_recv_idle_time += (TIM_IC_PERIOD_US / 1000);

            if(irda_data[idx].fsm_recv_idle_time >= 20)
                bsp_irda_post_data(&irda_data[idx]);
        }
    }

    if(TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

        bsp_irda_channelx_calc(1, &irda_data[0]);
    }

    if(TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);

        bsp_irda_channelx_calc(2, &irda_data[1]);
    }

    if(TIM_GetITStatus(TIM2, TIM_IT_CC3) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);

        bsp_irda_channelx_calc(3, &irda_data[2]);
    }

    if(TIM_GetITStatus(TIM2, TIM_IT_CC4) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC4);

        bsp_irda_channelx_calc(4, &irda_data[3]);
    }
}

void bsp_tim3_irda_channelx_calc(uint8_t channelx, struct IrDA_t *pdata)
{   
    if(pdata->ic_polarity == 0)
    {
        pdata->ic_polarity = 1;

        switch(channelx) //记录捕获时刻TIMx_CNT的值
        {
            case 1: pdata->ic_start = TIM_GetCapture1(TIM3); TIM_OC1PolarityConfig(TIM3, TIM_ICPolarity_Falling); break;
            case 2: pdata->ic_start = TIM_GetCapture2(TIM3); TIM_OC2PolarityConfig(TIM3, TIM_ICPolarity_Falling); break;
            default: break;
        }			
    }
    else
    {
        pdata->ic_polarity = 0;

        switch(channelx)//记录捕获时刻TIMx_CNT的值
        {
            case 1: pdata->ic_stop = TIM_GetCapture1(TIM3); TIM_OC1PolarityConfig(TIM3, TIM_ICPolarity_Rising); break;
            case 2: pdata->ic_stop = TIM_GetCapture2(TIM3); TIM_OC2PolarityConfig(TIM3, TIM_ICPolarity_Rising); break;        
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

void bsp_tim3_irda_isr(void)
{
    static struct IrDA_t irda_data[2];
    uint8_t idx = 0;

    // 节拍 10us
    // 周期 10 ms
	  
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        for(idx = 0; idx < 2; ++idx)
        {
            if(irda_data[idx].IrDA_FMS == IRDA_FMS_WAITLEADER) continue;

            irda_data[idx].fsm_recv_idle_time += (TIM_IC_PERIOD_US / 1000);

            if(irda_data[idx].fsm_recv_idle_time >= 20)
                bsp_irda_post_data(&irda_data[idx]);
        }
    }

    if(TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);

        bsp_tim3_irda_channelx_calc(1, &irda_data[0]);
    }

    if(TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);

        bsp_tim3_irda_channelx_calc(2, &irda_data[1]);
    }

}

#endif
