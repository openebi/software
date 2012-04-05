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
#include <inttypes.h>
#include <util/twi.h>
#include "twi.h"
#include "ad5933.h"

typedef struct {
    uint8_t msb;
    uint8_t lsb;
} ad5933_status_t;

ad5933_status_t ad5933_status = {0xA1, 0x00};

int ad5933_set_pointer(uint8_t paddr)
{
    int rv = -1;
    uint8_t buf[2] = {
        AD5933_CC_PADDR,
        paddr
    };

    if (twi_start(TWI_SLA_AD5933, TW_WRITE) != -1)
        if (twi_write(buf, 2) == 2)
            rv = 0;

    twi_stop();
    return rv;
}

/*  Read/Write a single byte
 * ------------------------------------------------------------------- */
uint8_t ad5933_rbyte(uint8_t raddr)
{
    uint8_t b = 0;

    if (ad5933_set_pointer(raddr) != -1)
        if (twi_start(TWI_SLA_AD5933, TW_READ) != -1)
            b = twi_read_byte();

    return b;
}

int ad5933_wbyte(uint8_t raddr, uint8_t b)
{
    int rv = -1;
    uint8_t buf[2] = {
        raddr,
        b
    };

    if (twi_start(TWI_SLA_AD5933, TW_WRITE) != -1)
        rv = twi_write(buf, 2);

    twi_stop();
    return rv;
}

/*  Read/Write block
 * ------------------------------------------------------------------- */
int ad5933_rblock(uint8_t raddr, uint8_t *buf, uint8_t n)
{
    int rv = 0;
    uint8_t buffer[2] = {
        AD5933_CC_RBLOCK,
        n
    };

    /* Set start address for a block write */
    if (ad5933_set_pointer(raddr) == -1)
        goto error;

    /* Init block read */
    if (twi_start(TWI_SLA_AD5933, TW_WRITE) == -1)
        goto error;
    if (twi_write(buffer, 2) != 2)
        goto error;
    // do not send stop here!

    /* Read block */
    if (twi_start(TWI_SLA_AD5933, TW_READ) == -1)
        goto error;
    rv = twi_read(buf, n);

quit:
    twi_stop();
    return rv;

error:
    rv = -1;
    goto quit;
}

int ad5933_wblock(uint8_t raddr, uint8_t *buf, uint8_t n)
{
    uint8_t buffer[n+2], m;
    int rv = 0;

    /* Set start address for a block write */
    if (ad5933_set_pointer(raddr) == -1)
        goto error;

    /* Init block write buffer */
    buffer[0] = AD5933_CC_WBLOCK;
    buffer[1] = n;
    for (m = 0; m < n; m++)
        buffer[m+2] = buf[m];

    /* Write block */
    if (twi_start(TWI_SLA_AD5933, TW_WRITE) == -1)
        goto error;
    rv = twi_write(buffer, n+2) - 2;

quit:
    twi_stop();
    return rv;

error:
    rv = -1;
    goto quit;
}

/*  Start frequency
 * ------------------------------------------------------------------- */
int ad5933_set_fstart(uint32_t f)
{
    uint8_t buf[3] = {
        (uint8_t) (f >> 16),
        (uint8_t) (f >> 8),
        (uint8_t) (f)
    };
    if (ad5933_wblock(AD5933_FREQRH, buf, 3) != 3)
        return -1;
    return 0;
}

unsigned long int ad5933_get_fstart(void)
{
    uint8_t buf[3];
    if (ad5933_rblock(AD5933_FREQRH, buf, 3) == 3)
        return ((((unsigned long int) buf[0] << 8) | buf[1]) << 8) | buf[2];
    return 0;
}

int ad5933_set_fstart_hz(double f)
{
    uint32_t code = ((f * 536870912) / AD5933_CLOCK_HZ);
    return ad5933_set_fstart(code);
}

/*  Frequency increment
 * ------------------------------------------------------------------- */
int ad5933_set_fincr(uint32_t f)
{
    uint8_t buf[3] = {
        (uint8_t) (f >> 16),
        (uint8_t) (f >> 8),
        (uint8_t) (f)
    };
    return ad5933_wblock(AD5933_FINCRH, buf, 3);
}

unsigned long int ad5933_get_fincr(void)
{
    uint8_t buf[3];
    if (ad5933_rblock(AD5933_FINCRH, buf, 3) == 3)
        return ((((unsigned long int) buf[0] << 8) | buf[1]) << 8) | buf[2];
    return 0;
}

int ad5933_set_fincr_hz(double f)
{
    uint32_t code = ((f * 536870912) / AD5933_CLOCK_HZ);
    return ad5933_set_fincr(code);
}

/*  Number of frequency increments
 * ------------------------------------------------------------------- */
int ad5933_set_nincr(uint16_t n)
{
    if (n > 511)
        n = 511;
    uint8_t buf[2] = {
        (uint8_t) (n >> 8),
        (uint8_t) n
    };
    return ad5933_wblock(AD5933_NINCRH, buf, 2);
}

unsigned int ad5933_get_nincr(void)
{
    uint8_t buf[2];
    if (ad5933_rblock(AD5933_NINCRH, buf, 2) == 2)
        return ((unsigned long int) buf[0] << 8) | buf[1];
    return 0;
}

int ad5933_set_tsettle(int n, uint8_t m)
{
    uint8_t buf[2];

    buf[2] = (uint8_t) n;
    if (m == 2)
        buf[1] = (buf[1] & 0xfB) | (1 << 1);
    if (m == 4)
        buf[1] = (1 << 1) | (1 << 2);
    else
        buf[1] &= 0xf9;

    return ad5933_wblock(AD5933_NCYCRH, buf, 2);
}

/*  Measurements
 * --------------------------------------------------------------------*/
int ad5933_get_real(void)
{
    uint8_t buf[2];
    if (ad5933_rblock(AD5933_REALDH, buf, 2) == 2)
        return (int) (((uint16_t) buf[0] << 8) | buf[1]);
    return 0;
}

int ad5933_get_imaginary(void)
{
    uint8_t buf[2];
    if (ad5933_rblock(AD5933_IMAGDH, buf, 2) == 2)
        return (int) (((uint16_t) buf[0] << 8) | buf[1]);
    return 0;
}

unsigned int ad5933_get_temperature(void)
{
    uint8_t buf[2];
    if (ad5933_rblock(AD5933_TEMPRH, buf, 2) == 2)
        return ((unsigned int) buf[0] << 8) | buf[1];
    return 0;
}

/*  Control functionts
 * --------------------------------------------------------------------*/
int ad5933_init_with_fstart(void)
{
    ad5933_status.msb = ((ad5933_status.msb & 0x0f) | (1 << 4));
    return ad5933_wbyte(AD5933_CTRLRH, ad5933_status.msb);
}

int ad5933_start_sweep(void)
{
    ad5933_status.msb = (ad5933_status.msb & 0x0f) | (1 << 5);
    return ad5933_wbyte(AD5933_CTRLRH, ad5933_status.msb);
}

int ad5933_increment_sweep(void)
{
    ad5933_status.msb = (ad5933_status.msb & 0x0f) | 0x30;
    return ad5933_wbyte(AD5933_CTRLRH, ad5933_status.msb);
}

int ad5933_sweep_complete(void)
{
    return ad5933_rbyte(AD5933_STATR) & AD5933_SWEEP_COMPLETE_MASK;
}

int ad5933_repeat_frequency(void)
{
    ad5933_status.msb = (ad5933_status.msb & 0x0f) | (1 << 6);
    return ad5933_wbyte(AD5933_CTRLRH, ad5933_status.msb);
}

int ad5933_meas_temperature(void)
{
    ad5933_status.msb = (ad5933_status.msb & 0x0f) | 0x90;
    return ad5933_wbyte(AD5933_CTRLRH, ad5933_status.msb);
}

int ad5933_has_valid_temperature(void)
{
    return (ad5933_rbyte(AD5933_STATR) & AD5933_VALID_TEMPERATURE_MASK);
}

int ad5933_has_valid_impedance(void)
{
    return (ad5933_rbyte(AD5933_STATR) & AD5933_VALID_IMPEDANCE_MASK);
}

int ad5933_set_output_range(uint8_t range)
{
    switch (range) {
        case 1:
            ad5933_status.msb &= 0xf1;
            break;
        case 4:
            ad5933_status.msb = (ad5933_status.msb & 0xf1) | (1 << 1);
            break;
        case 3:
            ad5933_status.msb = (ad5933_status.msb & 0xf1) | (1 << 2);
            break;
        case 2:
            ad5933_status.msb = (ad5933_status.msb & 0xf1) | (1 << 1) | (1 << 2);
            break;
    }
    return ad5933_wbyte(AD5933_CTRLRH, ad5933_status.msb);
}

int ad5933_set_pga_gain(bool enabled)
{
    if (enabled) {
        ad5933_status.msb |= 1;
    } else {
        ad5933_status.msb &= ~1;
    }
    return ad5933_wbyte(AD5933_CTRLRH, ad5933_status.msb);
}

int ad5933_standby(void)
{
    ad5933_status.msb = (ad5933_status.msb & 0x0f) | 0xB0;
    return ad5933_wbyte(AD5933_CTRLRH, ad5933_status.msb);
}

int ad5933_pwrdown(void)
{
    ad5933_status.msb = (ad5933_status.msb & 0x0f) | (1 << 7) | (1 << 5);
    return ad5933_wbyte(AD5933_CTRLRH, ad5933_status.msb);
}

int ad5933_reset(void)
{
    ad5933_status.lsb &= 0x18;
    return ad5933_wbyte(AD5933_CTRLRL, ad5933_status.lsb | (1 << 4));
}

