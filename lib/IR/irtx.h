#ifndef __IRTX_H
#define __IRTX_H

#include "main.h"

void IR_PostData(uint8_t data);
void IR_HIGH(uint16_t hold_time_us);
void IR_LOW(uint16_t hold_time_us);
void Send_ByteData(uint8_t byte_data);
#endif