/*
 * temp.c
 *
 *  Created on: Jun 6, 2025
 *      Author: hp15s-pc
 */
#include"temp.h"

void set_pin_output(void)
{
    // 1. Set MODER[2n:2n+1] = 01 (General purpose output)
    DHT_PORT->MODER &= ~(0x3 << (DHT_PIN * 2));
    DHT_PORT->MODER |=  (0x1 << (DHT_PIN * 2));

    // 2. Set OTYPER[n] = 0 (Push-pull)
    DHT_PORT->OTYPER &= ~(1 << DHT_PIN);

    // 3. Set OSPEEDR[2n:2n+1] = 01 (Medium speed ~10 MHz)
    DHT_PORT->OSPEEDR &= ~(0x3 << (DHT_PIN * 2));
    DHT_PORT->OSPEEDR |=  (0x1 << (DHT_PIN * 2));

    // 4. Optional: No pull-up/down
    DHT_PORT->PUPDR &= ~(0x3 << (DHT_PIN * 2));
}
void set_pin_input(void)
{
    // 1. Set MODER[2n:2n+1] = 00 (Input mode)
    DHT_PORT->MODER &= ~(0x3 << (DHT_PIN * 2));

    // 2. No pull-up/pull-down (floating input)
    DHT_PORT->PUPDR &= ~(0x3 << (DHT_PIN * 2));
}

void DHT_start(void)
{
    set_pin_output();
    DHT_PORT->ODR &= ~(1 << DHT_PIN);   // Pull LOW
    delay_ms(20);                       // ≥18ms
    DHT_PORT->ODR |= (1 << DHT_PIN);    // Pull HIGH
    delay_us(30);                       // 20-40us
    set_pin_input();                    // Release line
}

int8_t DHT_check_response(void)
{
    delay_us(40);
    if (!(DHT_PORT->IDR & (1 << DHT_PIN))) // LOW for 80us
    {
        delay_us(80);
        if (DHT_PORT->IDR & (1 << DHT_PIN)) // HIGH for 80us
        {
            delay_us(50);
            return 1;
        }
    }
    return 0;
}

uint8_t DHT_read_byte(void)
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++)
    {
        while (!(DHT_PORT->IDR & (1 << DHT_PIN))); // Wait for HIGH
        delay_us(40);
        if (DHT_PORT->IDR & (1 << DHT_PIN))
            byte |= (1 << (7 - i)); // If HIGH after 40us → 1
        while (DHT_PORT->IDR & (1 << DHT_PIN)); // Wait for LOW
    }
    return byte;
}

uint8_t DHT_read(uint8_t* temp, uint8_t* hum)
{
    uint8_t hum_int, hum_dec, temp_int, temp_dec, checksum;

    DHT_start();
    if (!DHT_check_response())
        return 0;

    hum_int = DHT_read_byte();
    hum_dec = DHT_read_byte();
    temp_int = DHT_read_byte();
    temp_dec = DHT_read_byte();
    checksum = DHT_read_byte();

    if (checksum == (hum_int + hum_dec + temp_int + temp_dec))
    {
        *temp = temp_int;
        *hum = hum_int;
        return 1;
    }
    return 0;
}

void delay_us(uint32_t us)
{
    us *= 8;  // Rough for 72MHz (adjust if needed)
    while (us--) __NOP();
}

void delay_ms(uint32_t ms)
{
    while (ms--) delay_us(1000);
}
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
