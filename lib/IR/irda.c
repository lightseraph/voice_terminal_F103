#include "irda.h"

uint8_t g_remote_sta = 0;
uint32_t g_remote_data = 0; /* 红外接收到的数据 */
uint8_t g_remote_cnt = 0;   /* 按键按下的次数 */
uint8_t g_valid_channel = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim3)
    {
        if (g_remote_sta & 0x80) /* 上次有数据被接收到了 */
        {
            g_remote_sta &= ~0X10; /* 取消上升沿已经被捕获标记 */

            if ((g_remote_sta & 0X0F) == 0X00)
            {
                g_remote_sta |= 1 << 6; /* 标记已经完成一次按键的键值信息采集 */
            }

            if ((g_remote_sta & 0X0F) < 14)
            {
                g_remote_sta++;
            }
            else
            {
                g_remote_sta &= ~(1 << 7); /* 清空引导标识 */
                g_remote_sta &= 0XF0;      /* 清空计数器 */
            }
        }
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim3)
    {
        if (g_valid_channel != 2)
        {
            if (IR_A_DATA)
            {
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
                __HAL_TIM_SET_COUNTER(htim, 0);
                g_remote_sta |= 0x10;
                g_valid_channel = 1;
            }
            else
            {
                DataCollect(htim);
            }
        }
        else if (g_valid_channel != 1)
        {
            if (IR_B_DATA)
            {
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING);
                __HAL_TIM_SET_COUNTER(htim, 0);
                g_remote_sta |= 0x10;
                g_valid_channel = 2;
            }
            else
            {
                DataCollect(htim);
            }
        }
    }
}

uint8_t remote_scan(void)
{
    uint8_t sta = 0;
    uint8_t t1, t2;

    if (g_remote_sta & (1 << 6)) /* 得到一个按键的所有信息了 */
    {
        t1 = g_remote_data;               /* 得到地址码 */
        t2 = (g_remote_data >> 8) & 0xff; /* 得到地址反码 */

        if ((t1 == (uint8_t)~t2) && t1 == REMOTE_ID) /* 检验遥控识别码(ID)及地址 */
        {
            t1 = (g_remote_data >> 16) & 0xff;
            t2 = (g_remote_data >> 24) & 0xff;

            if (t1 == (uint8_t)~t2)
            {
                sta = t1; /* 键值正确 */
                g_remote_data = 0;
                g_remote_sta = 0;
                g_valid_channel = 0;
            }
        }

        if ((sta == 0) || ((g_remote_sta & 0X80) == 0)) /* 按键数据错误/遥控已经没有按下了 */
        {
            g_remote_sta &= ~(1 << 6); /* 清除接收到有效按键标识 */
            g_remote_cnt = 0;          /* 清除按键次数计数器 */
        }
    }

    return sta;
}

void DataCollect(TIM_HandleTypeDef *htim)
{
    uint16_t dval;
    if (g_valid_channel != 2)
    {
        dval = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);                               /* 读取CCR1也可以清CC1IF标志位 */
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING); /* 设置为上升沿捕获 */
    }
    else if (g_valid_channel != 1)
    {
        dval = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);                               /* 读取CCR1也可以清CC1IF标志位 */
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING); /* 设置为上升沿捕获 */
    }
    if (g_remote_sta & 0X10) /* 完成一次高电平捕获 */
    {
        if (g_remote_sta & 0X80) /* 接收到了引导码 */
        {

            if (dval > 400 && dval < 700) /* 560为标准值,560us */
            {
                g_remote_data >>= 1;            /* 右移一位 */
                g_remote_data &= ~(0x80000000); /* 接收到0 */
            }
            else if (dval > 1400 && dval < 1800) /* 1680为标准值,1680us */
            {
                g_remote_data >>= 1;         /* 右移一位 */
                g_remote_data |= 0x80000000; /* 接收到1 */
            }
            else if (dval > 2200 && dval < 2800) /* 得到按键键值增加的信息 2500为标准值2.5ms */
            {
                g_remote_cnt++;       /* 按键次数增加1次 */
                g_remote_sta &= 0XF0; /* 清空计时器 */
            }
        }
        else if (dval > 4200 && dval < 4700) /* 4500为标准值4.5ms */
        {
            g_remote_sta |= 1 << 7; /* 标记成功接收到了引导码 */
            g_remote_cnt = 0;       /* 清除按键次数计数器 */
        }
    }

    g_remote_sta &= ~(1 << 4);
}