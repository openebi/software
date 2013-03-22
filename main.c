/**
 * OpenEBI
 *
 * Copyright (c) 2012-2013 Kim H Blomqvist
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

#include "board.h"
#include "usart0.h"
#include "ad5933.h"

// #define __ASSERT_USE_STDERR 1
// #include <assert.h> // diagnostics for unit tests

// void run_tests(void)
// {
//     uint8_t b;
//     unsigned int nincr;
//     unsigned long int freq, fincr;

//     assert(ad5933_wbyte(AD5933_FREQRL, 101) != -1);    putchar('.');
//     b = ad5933_rbyte(AD5933_FREQRL);
//     assert(b == 101);                                  putchar('.');

//     assert(ad5933_set_fstart(94928) != -1);            putchar('.');
//     freq = ad5933_get_fstart();
//     assert(freq == 94928);                             putchar('.');

//     assert(ad5933_set_fincr(48924) != -1);             putchar('.');
//     fincr = ad5933_get_fincr();
//     assert(fincr == 48924);                            putchar('.');

//     assert(ad5933_set_nincr(0) != -1);                 putchar('.');
//     nincr = ad5933_get_nincr();
//     assert(nincr == 0);                                putchar('.');

//     printf("OK!\n");
//     return;
// }

typedef struct SweepOptions SweepOptions;

struct SweepOptions {
    double fstart;
    double fincr;
    uint16_t nincr;
    uint16_t tsettle;
    uint8_t xtsettle;
    uint8_t nrange;
    uint8_t pgagain;
    uint8_t average;
};

void init_ad5933(SweepOptions *o)
{
    ad5933_reset();
    ad5933_set_fstart_hz(o->fstart);
    ad5933_set_nincr(o->nincr);
    ad5933_set_fincr_hz(o->fincr);
    ad5933_set_tsettle(o->tsettle, o->xtsettle);
    ad5933_set_output_range(o->nrange);
    ad5933_set_pga_gain(o->pgagain);
    ad5933_standby();
}

void take_measurement(uint8_t avg, double *rdata, double *idata)
{
    int i;
    int32_t rdata_raw = 0, idata_raw = 0;

    for (i = 0; i < avg; i++) {
        _delay_ms(1); // The conversion process takes approximately 1 ms using a 16.777 MHz clock.
        while (!ad5933_has_valid_impedance());
        rdata_raw += ad5933_get_real();
        idata_raw += ad5933_get_imaginary();

        if (avg > 1) {
            ad5933_repeat_frequency();
        }
    }

    *rdata = (double) rdata_raw / avg;
    *idata = (double) idata_raw / avg;
}

void sweep(FILE *stream, uint8_t average)
{
    double rdata, idata;

    ad5933_init_with_fstart();
    ad5933_start_sweep();

    take_measurement(average, &rdata, &idata);
    fprintf(stream, "%.4f %.4f\n", rdata, idata);
    ad5933_increment_sweep();

    while (!ad5933_sweep_complete()) {
        take_measurement(average, &rdata, &idata);
        fprintf(stream, "%.4f %.4f\n", rdata, idata);
        ad5933_increment_sweep();
    }
    ad5933_reset();
}

void freerun(FILE *stream)
{
    ad5933_init_with_fstart();
    ad5933_start_sweep();

    while(!USART0_ESCAPE) {
        _delay_ms(1); // The conversion process takes approximately 1 ms using a 16.777 MHz clock.
        while (!ad5933_has_valid_impedance());
        fprintf(stream, "%d %d\n", ad5933_get_real(), ad5933_get_imaginary());
        ad5933_repeat_frequency();
    }
    ad5933_reset();
}

void print_options(FILE *stream, SweepOptions *o)
{
    fprintf(stream, "-fstart   = %.2lf\n", o->fstart);
    fprintf(stream, "-fincr    = %.2lf\n", o->fincr);
    fprintf(stream, "-nincr    = %u\n", o->nincr);
    fprintf(stream, "-tsettle  = %u\n", o->tsettle);
    fprintf(stream, "-xtsettle = %hhu\n", o->xtsettle);
    fprintf(stream, "-nrange   = %hhu\n", o->nrange);
    fprintf(stream, "-pgagain  = %s\n", o->pgagain ? "true" : "false");
    fprintf(stream, "-average  = %hhu\n", o->average);
}

#define VERSION "v0.2"

int main(void)
{
    SweepOptions opts = {
        .fstart = 4000,
        .fincr = 2000,
        .nincr = 48,
        .tsettle = 10,
        .xtsettle = 1,
        .nrange = 1,
        .pgagain = true,
        .average = 16
    };

    char cmdbuf[64] = {};

    init_board();
    init_ad5933(&opts);

    _delay_ms(1000);
    printf("\rOpenEBI " VERSION "\n");
    printf("Copyright (c) 2012-2013 Kim H Blomqvist\n");
    printf("Developed at the Department of Electronics at Aalto University.\n\n");
    print_options(stdout, &opts);
    printf("\nWhile in PuTTY use ^J instead of ENTER\n\n");

    /*  Main loop */
    for (;;) {
        printf("$ ");
        if (fgets(cmdbuf, sizeof(cmdbuf), stdin) == NULL) {
            printf("ERROR!\n");
            continue;
        }

        switch (cmdbuf[0]) {
            case 's':
                sweep(stdout, opts.average);
                break;
            case 'p':
                sscanf(&cmdbuf[1], "%lf %lf %u %u %hhu %hhu %hhu %hhu", &opts.fstart, &opts.fincr,
                    &opts.nincr, &opts.tsettle, &opts.xtsettle, &opts.nrange, &opts.pgagain, &opts.average);
                if (opts.tsettle > 511)
                    opts.tsettle = 511;
                if (opts.xtsettle != 1 && opts.xtsettle != 2 && opts.xtsettle != 4)
                    opts.xtsettle = 1;
                init_ad5933(&opts);
                break;
            case 'f':
                freerun(stdout);
                break;
            case 'o':
                print_options(stdout, &opts);
                break;
            // case 't':
            //     run_tests();
            //     break;
            case 'h':
                printf(
                    "OpenEBI " VERSION "\n"
                    "Copyright (c) 2012-2013 Kim H Blomqvist\n"
                    "Developed at the Department of Electronics at Aalto University.\n\n"
                    "s\tRuns a frequency sweep. Output is in \"R I\" format.\n"
                    "p\tSets sweep options. The argument order is as in options struct.\n"
                    "f\tFreerun using the programmed start frequency. Abort with ESC.\n"
                    "o\tPrints the current options.\n"
                    // "t\tRuns unit tests.\n"
                    "h\tShows this help.\n"
                );
                break;
            default:
                printf("Command 'h' for help\n");
                break;
        }
    }
}
