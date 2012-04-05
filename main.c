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
#ifndef F_CPU
#error Define F_CPU (CPU frequency) in your makefile
#endif

#include <util/delay.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>

/* For unit tests
#define __ASSERT_USE_STDERR 1
#include <assert.h> // diagnostics for unit tests
*/

#include "board.h"
#include "usart0.h"
#include "ad5933.h"

typedef struct SweepOptions SweepOptions;

struct SweepOptions {
    double fstart;
    double fincr;
    uint16_t nincr;
    uint8_t range;
    bool pgagain;
    uint8_t average;
};

void init_sweep(SweepOptions *o)
{
    ad5933_reset();
    ad5933_set_fstart_hz(o->fstart);
    ad5933_set_nincr(o->nincr);
    ad5933_set_fincr_hz(o->fincr);
    ad5933_set_tsettle(511, 4);
    ad5933_set_output_range(o->range);
    ad5933_set_pga_gain(o->pgagain);
    ad5933_standby();
}

void take_measurement(uint8_t avg, double *rdata, double *idata)
{
    int i;
    int32_t rdata_raw = 0, idata_raw = 0;

    for (i = 0; i < avg; i++) {
        _delay_ms(1);
        while (!ad5933_has_valid_impedance());
        rdata_raw += ad5933_get_real();
        idata_raw += ad5933_get_imaginary();

        if (avg > 1) {
            ad5933_repeat_frequency();
        }
    }

    *rdata = (double) rdata_raw/avg;
    *idata = (double) idata_raw/avg;
}

void sweep(FILE *stream, SweepOptions *o)
{
    double rdata, idata;

    ad5933_init_with_fstart();
    ad5933_start_sweep();

    take_measurement(o->average, &rdata, &idata);
    fprintf(stream, "%4.2f;%4.2f\r\n", rdata, idata);
    ad5933_increment_sweep();

    while (!ad5933_sweep_complete()) {
        take_measurement(o->average, &rdata, &idata);
        fprintf(stream, "%4.2f;%4.2f\r\n", rdata, idata);
        ad5933_increment_sweep();
    }
    ad5933_reset();
}

void freerun(FILE *stream)
{
    double rdata, idata;

    ad5933_init_with_fstart();
    ad5933_start_sweep();

    while(!USART0_ESCAPE) {
        take_measurement(1, &rdata, &idata);
        fprintf(stream, "%4.2f\t%4.2f\r\n", rdata, idata);
        ad5933_repeat_frequency();
    }
    ad5933_reset();
}

void print_options(SweepOptions *o)
{
    printf("\rOptions:\r\n");
    printf("-fstart  = %.2lf\r\n", o->fstart);
    printf("-fincr   = %.2lf\r\n", o->fincr);
    printf("-nincr   = %u\r\n", o->nincr);
    printf("-range   = %u\r\n", o->range);
    printf("-average = %u\r\n", o->average);
}

#define VERSION "v0.1"

SweepOptions _opts = {
    .fstart = 4000,
    .fincr = 2000,
    .nincr = 48,
    .range = 1,
    .pgagain = true,
    .average = 16
};

#define COMMAND_BUFFER 30
char _cmdbuf[COMMAND_BUFFER] = {};

int main(void)
{
    init_board();
    init_sweep(&_opts);

    _delay_ms(1000);
    printf("\r\nOpenEBI - Open Electrical Bioimpedance " VERSION "\r\n");
    print_options(&_opts);
    printf("\r\nWhile in putty use ^J instead of ENTER\r\n");

    /*  Main loop */
    for (;;) {
        printf("\r$ ");
        if (fgets(_cmdbuf, COMMAND_BUFFER - 1, stdin) == NULL) {
            printf("ERROR!\r\n");
            break;
        }

        switch (_cmdbuf[0]) {
            case 's':
                sscanf(&_cmdbuf, "s:%u", &_opts.average);
                printf("\r");
                sweep(stdout, &_opts);
                break;
            case 'f':
                printf("\r\n");
                freerun(stdout);
                break;
            /* For some unkown reason does not work
            case 'p':
                sscanf(&_cmdbuf[2], "%lf:%lf:%u",
                    &_opts.fstart,
                    &_opts.fincr,
                    &_opts.nincr
                );
                init_sweep(&_opts);
            */
            case 'o':
                print_options(&_opts);
                break;
            default:
                printf("\rCommand 'h' for help\r\n");
                break;
        }
    }
}

/* Unit tests
void run_tests(void)
{
    uint8_t b;
    unsigned int nincr;
    unsigned long int freq, fincr;

    assert(ad5933_wbyte(AD5933_FREQRL, 101) != -1);    putchar('.');
    b = ad5933_rbyte(AD5933_FREQRL);
    assert(b == 101);                                  putchar('.');

    assert(ad5933_set_fstart(94928) != -1);            putchar('.');
    freq = ad5933_get_fstart();
    assert(freq == 94928);                             putchar('.');

    assert(ad5933_set_fincr(48924) != -1);             putchar('.');
    fincr = ad5933_get_fincr();
    assert(fincr == 48924);                            putchar('.');

    assert(ad5933_set_nincr(0) != -1);               putchar('.');
    nincr = ad5933_get_nincr();
    assert(nincr == 0);                              putchar('.');

    printf("OK!");
    return;
}
*/