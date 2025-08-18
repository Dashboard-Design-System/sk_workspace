/*
 * ir.c
 *
 *  Created on: Jun 14, 2025
 *      Author: sunbeam
 */


#include "ir.h"


void ir_init(void)
{

	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	//led config

	GPIOD->MODER &= ~(BV(GREEN_LED *2 +1));
	GPIOD->MODER |= BV(GREEN_LED * 2);

	GPIOD->PUPDR &= ~(BV(GREEN_LED *2 +1) | BV(GREEN_LED*2));
	GPIOD->OSPEEDR &= ~(BV(GREEN_LED*2  +1) | BV(GREEN_LED*2));


	GPIOA->MODER &= ~(BV(IR_SENSOR *2 +1 ) | BV(IR_SENSOR));
	GPIOA ->PUPDR &= ~(BV(IR_SENSOR * 2 +1 ) | BV(IR_SENSOR * 2));

}


void led_on()
{
	GPIOD->ODR |= BV(GREEN_LED);
}

void led_off()
{
	GPIOD->ODR &= ~(BV(GREEN_LED));
}

uint16_t ir_detect()
{

	return GPIOA->IDR & BV(IR_SENSOR) ? 1:0;
}

