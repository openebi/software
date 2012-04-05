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
#ifndef __TWI_FUNCS_H
#define __TWI_FUNCS_H 1

#define TWI_MAX_ITER 100
#define TWI_WAIT_FOR_TX() do {} while ((TWCR & _BV(TWINT)) == 0)

/**
 * Send start condition
 *
 * \return 0 on success and -1 on error
 */
int twi_sstart(void);

/**
 * Start R/~W condition
 *
 * Internally sends start condition by calling twi_sstart().
 *
 * \param addr Serial bus address of the device. !! LSB has to be reserved for
 *             R/W bit so left shift the address by one.
 * \param rwbit R/W process, Read = TW_READ, Write = TW_WRITE
 */
int twi_start(uint8_t addr, int rwbit);

/**
 * Writes sequence of bytes to bus
 *
 * This function leaves bus open for further byte transmission, so when
 * you are finished remember to call twi_stop() to release the bus.
 *
 * !! Before writing any byte remember to address your write by calling
 *    twi_start(addr, TW_WRITE).
 *
 * \param buf Pointer to the write buffer
 * \param n   Number of bytes to write
 */
int twi_write(uint8_t *buf, int n);

int twi_write_byte(uint8_t byte);

int twi_read(uint8_t *buf, int n);

uint8_t twi_read_byte(void);

/**
 * Write stop condition
 */
void twi_stop(void);

/**
 * Write repeat start condition
 */
void twi_rstart(void);

#endif

