
#ifndef __BSP_IRDA_H_
#define __BSP_IRDA_H_

#include "user_main.h"


#if (CFG_DEVICE_MODE == DEVICE_MODE_IRDA_SEND)
void bsp_irda_send_config(void);
#elif (CFG_DEVICE_MODE == DEVICE_MODE_MOBILE)
void bsp_irda_recv_config(void);
#endif

#if (CFG_DEVICE_MODE == DEVICE_MODE_IRDA_SEND)
void bsp_irda_send_enable(void);
void bsp_irda_send_disable(void);

#elif (CFG_DEVICE_MODE == DEVICE_MODE_MOBILE)

void bsp_irda_isr(void);
void bsp_tim3_irda_recv_config(void);
void bsp_tim3_irda_isr(void);

#endif

#endif /* end of __BSP_IRDA_H_ */
