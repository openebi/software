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
#ifndef __AD5933_H
#define __AD5933_H

#include <stdbool.h>
#include <inttypes.h>
#include <util/twi.h>

/*  Default serial bus address, data sheet p. 28.
 *  Remember to shift it by one to make a space for R/W bit. */
#ifndef TWI_SLA_AD5933
    #define TWI_SLA_AD5933 (0x0D << 1)
#endif

#define AD5933_CLOCK_HZ 16776000

#define AD5933_CTRLRH 0x80 // control register high
#define AD5933_CTRLRL 0x81 // control register low

#define AD5933_FREQRH 0x82 // start frequency register high
#define AD5933_FREQRM 0x83 // start freq. register median
#define AD5933_FREQRL 0x84 // start freq. register low

#define AD5933_FINCRH 0x85 // frequency increment register high
#define AD5933_FINCRM 0x86 // freq. incr. register median
#define AD5933_FINCRL 0x87 // freq. incr. register low

#define AD5933_NINCRH 0x88 // number of increments register high
#define AD5933_NINCRL 0x89 // num. of incr. register low

#define AD5933_NCYCRH 0x8A // number of settling time cycles register high
#define AD5933_NCYCRL 0x8B // num. of settling time cycles register low

#define AD5933_STATR  0x8F // status register

#define AD5933_TEMPRH 0x92 // temperature register high
#define AD5933_TEMPRL 0x93 // temperature register low

#define AD5933_REALDH 0x94 // real data register high
#define AD5933_REALDL 0x95 // real data registger low

#define AD5933_IMAGDH 0x96 // imaginary data register high
#define AD5933_IMAGDL 0x97 // imaginary data register low

#define AD5933_CC_WBLOCK 0xA0
#define AD5933_CC_RBLOCK 0xA1
#define AD5933_CC_PADDR 0xB0

#define AD5933_SWEEP_COMPLETE_MASK 0x04
#define AD5933_VALID_IMPEDANCE_MASK 0x02
#define AD5933_VALID_TEMPERATURE_MASK 0x01

/*  Set AD5933's pointer to point paddr */
int ad5933_set_pointer(uint8_t paddr);

/*  Read and write single byte into register address (raddr) */
int ad5933_wbyte(uint8_t raddr, uint8_t b);
uint8_t ad5933_rbyte(uint8_t raddr);

/*  Read and write block starting from register address (raddr).
 *  Returns number of bytes read or write. */
int ad5933_wblock(uint8_t raddr, uint8_t *buf, uint8_t n);
int ad5933_rblock(uint8_t raddr, uint8_t *buf, uint8_t n);

/*  Set and get 24-bit start frequency code. Setter returns -1 on error. */
int ad5933_set_fstart(uint32_t f);
unsigned long int ad5933_get_fstart(void);
int ad5933_set_fstart_hz(double f);

/*  Set and get 24-bit frequency increment code. Setter returns -1 on error */
int ad5933_set_fincr(unsigned long int i);
unsigned long int ad5933_get_fincr(void);
int ad5933_set_fincr_hz(double f);

/*  Set and get number of frequency increments. Setter returns -1 on error */
int ad5933_set_nincr(uint16_t n);
unsigned int ad5933_get_nincr(void);

int ad5933_set_tsettle(int n, uint8_t m);
//int ad5933_get_tsettle(void);

/*  Set output range no. Check datasheet p. 22
 *  1 = 2.0 Vpp, 2 = 1.0 Vpp, 3 = 400mVpp, 4 = 200mVpp */
int ad5933_set_output_range(uint8_t range);
int ad5933_set_pga_gain(bool enabled);

int ad5933_init_with_fstart(void);
int ad5933_start_sweep(void);
int ad5933_increment_sweep(void);
int ad5933_repeat_frequency(void);

int ad5933_meas_temperature(void);
int ad5933_has_valid_impedance(void);
int ad5933_has_valid_temperature(void);
int ad5933_sweep_complete(void);

int ad5933_standby(void);
int ad5933_pwrdown(void);
int ad5933_reset(void);

unsigned int ad5933_get_temperature(void);
int ad5933_get_real(void);
int ad5933_get_imaginary(void);

#endif

