/**
 * OpenEBI
 *
 * The base of twi functions have been adapted from an example code of
 * avrlibc written by <joerg@FreeBSD.ORG> and licensed under "THE BEER-WARE
 * LICENSE" (Revision 42) [1]. Those functions have been refactored here
 * to provide more convenient and general api for twi communication.
 *
 * [1] http://www.nongnu.org/avr-libc/user-manual/group__twi__demo.html
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

uint8_t twi_status;

int twi_sstart(void)
{
begin:

    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
    TWI_WAIT_FOR_TX(); /* wait for transmission */

    switch (twi_status = TW_STATUS) {
        case TW_REP_START:
        case TW_START:
            break;
        case TW_MT_ARB_LOST:
            goto begin;
        case TW_BUS_ERROR:
        default:
            return -1; /* error: not in start condition */
    }
    return 0;
}

int twi_start(uint8_t addr, int rwbit)
{
    uint8_t n = 0;

restart:

    if (n++ >= TWI_MAX_ITER)
        return -1;

begin:

    if (twi_sstart() == -1) /* send start condition */
        return -1;

    if (rwbit == TW_WRITE)
        TWDR = addr | TW_WRITE; /*  Send SLA+W */
    else if (rwbit == TW_READ)
        TWDR = addr | TW_READ; /*  Send SLA+R */
    else
        return -1;

    TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
    TWI_WAIT_FOR_TX();

    switch (twi_status = TW_STATUS) {
        case TW_MT_SLA_ACK:
            if (rwbit != TW_WRITE)
                goto error;
            break;
        case TW_MT_SLA_NACK:
            if (rwbit != TW_WRITE)
                goto error;
            goto restart;
        case TW_MR_SLA_ACK:
            if (rwbit != TW_READ)
                goto error;
            break;
        case TW_MR_SLA_NACK:
            if (rwbit != TW_READ)
                goto error;
            goto restart;
        case TW_MT_ARB_LOST:
        //case TW_MR_ARB_LOST: /* same as TW_MT_ARB_LOST */
            goto begin;
        case TW_BUS_ERROR:
        default:
            goto error;
    }

    return 1;

error:
    /* error: must send stop condition */
    twi_stop();
    return -1;
}

int twi_write(uint8_t *buf, int n)
{
    int rv = 0; /* return value */

    while (rv < n) {
        TWDR = buf[rv];
        TWCR = _BV(TWINT) | _BV(TWEN);
        TWI_WAIT_FOR_TX();

        switch (twi_status = TW_STATUS) {
            case TW_MT_DATA_ACK:
                rv++; /* byte transmission ok */
                break;

            case TW_MT_DATA_NACK:
            default:
                n = 0;
                rv = -1;
        }
    }

    return rv;
}

int twi_write_byte(uint8_t b)
{
    return twi_write(&b, 1);
}

uint8_t twi_read_byte(void)
{
    uint8_t b;
    twi_read(&b, 1);
    return b;
}

int twi_read(uint8_t *buf, int n)
{
    int rv = 0; /* return value */

    while (rv < n) {
        if (rv < (n-1))
            TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
        else
            TWCR = _BV(TWINT) | _BV(TWEN);
        TWI_WAIT_FOR_TX();

        switch ((twi_status = TW_STATUS)) {
            case TW_MR_DATA_NACK:
                n = 0; /* last byte transmitted, fall through */
            case TW_MR_DATA_ACK:
                *buf++ = TWDR;
                rv++;
                break;
            default:
                return -1;
                twi_stop();
        }
    }

    return rv;
}

void twi_stop(void)
{
    TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
}

void twi_rstart(void)
{
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
}
