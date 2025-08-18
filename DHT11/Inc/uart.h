/*
 * uart.h
 *
 *  Created on: Mar 29, 2025
 *      Author: admin
 */

#ifndef UART_H_
#define UART_H_

#include "stm32f4xx.h"

#ifndef BV
#define BV(n) (1<<(n))
#endif

void UartInit(uint32_t baud);
void UartPuts(char *str);

#endif /* UART_H_ */
