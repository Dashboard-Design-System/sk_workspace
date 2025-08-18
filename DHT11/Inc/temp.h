/*
 * temp.h
 *
 *  Created on: Jun 6, 2025
 *      Author: hp15s-pc
 */

#ifndef TEMP_H_
#define TEMP_H_

#include "stm32f407xx.h"   // Use your CMSIS device header



#define DHT_PORT GPIOA
#define DHT_PIN  1

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void set_pin_output(void);
void set_pin_input(void);
void DHT_start(void);
uint8_t DHT_read_byte(void);
uint8_t DHT_read(uint8_t* temp, uint8_t* hum);

#endif /* TEMP_H_ */
