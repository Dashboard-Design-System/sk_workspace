/*
 * ir.h
 *
 *  Created on: Jun 14, 2025
 *      Author: sunbeam
 */

#ifndef IR_H_
#define IR_H_

#include<stm32f4xx.h>

#define GREEN_LED  12
#define IR_SENSOR  0

void ir_init(void);

void led_on();
void led_off();

uint16_t ir_detect();
#endif /* IR_H_ */
