#include "main.h"
#include "tim.h"

#define REMOTE_INA_GPIO_PORT GPIOA
#define REMOTE_INB_GPIO_PORT GPIOA
#define REMOTE_INA_GPIO_PIN GPIO_PIN_6
#define REMOTE_INB_GPIO_PIN GPIO_PIN_7

#define IR_A_DATA HAL_GPIO_ReadPin(REMOTE_INA_GPIO_PORT, REMOTE_INA_GPIO_PIN)
#define IR_B_DATA HAL_GPIO_ReadPin(REMOTE_INB_GPIO_PORT, REMOTE_INB_GPIO_PIN)

#define REMOTE_ID 0xBF

extern uint8_t g_remote_sta;
extern uint32_t g_remote_data;  /* 红外接收到的数据 */
extern uint8_t g_remote_cnt;    /* 按键按下的次数 */
extern uint8_t g_valid_channel; /* 当前捕获通道 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
uint8_t remote_scan(void);
void DataCollect(TIM_HandleTypeDef *htim);