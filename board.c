/**
 * OpenEBI
 *
 * Copyright (c) 2012 Kim H Blomqvist
 * Developed at the Department of Electronics at Aalto University
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <avr/io.h>
#include <avr/power.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include "usart0.h"
#include "board.h"

/*  Setup streams for communication via usart */
static FILE usart_stream = FDEV_SETUP_STREAM(
    usart0_putchar, usart0_getchar, _FDEV_SETUP_RW);

void init_board(void)
{
    /*  Initialize general io pins */
    DDRB = 0xff;
    PORTB = 0x00;

    /*  Make sure that only necessary modules are powered up. */
    power_all_disable();
    power_usart0_enable();
    power_twi_enable();

    /*  Initialize usart and declare standard input and output streams */
    init_usart0();
    stdin = stdout = &usart_stream;
    stderr = stdout;

    init_twi();
}


/*  Usart
 * --------------------------------------------------------------------- */
#define BAUD      19200   // baud rate, see <util/setbaud.h>
#define BOUD_TOL  2       // baud tolerance 2%, see <util/setbaud.h>
#define USE_2X    0       // don't use prescaler, see <util/setbaud.h>
#include <util/setbaud.h> // helper macros for baud rate calculations

#define USART_PORT DDRD   // usart port
#define USART_RX   DDD0   // usart rx pin
#define USART_TX   DDD1   // usart tx pin

void init_usart0(void)
{
    /*  Set baud rate using avr-libc helper macros */
    UBRR0L = UBRRL_VALUE;
    UBRR0H = UBRRH_VALUE;
    
    /*  Asynchronous USART, Parity = none, Stop bits = 1, Data bits = 8 */
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    
    /*  Enable RX and TX */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);

    /*  Override general io pins for usart rx/tx */
    USART_PORT &= ~_BV(USART_RX);
    USART_PORT |= _BV(USART_TX);
}


/*  Twi
 * --------------------------------------------------------------------- */
#define TWI_SDA  DDC4
#define TWI_SCL  DDC5

void init_twi(void)
{
    /*  TWI clock 100 kHz. Prescaler (in TWSR register) has initial value 1 */
    TWBR = (F_CPU / 100000UL - 16) / 2;

    /*  Port pin configuration; i/o = output, state = high, pull-up = no */
    DDRC  |= _BV(TWI_SDA) & _BV(TWI_SCL);
    PORTC |= _BV(PINC4) & _BV(PINC5);
}

