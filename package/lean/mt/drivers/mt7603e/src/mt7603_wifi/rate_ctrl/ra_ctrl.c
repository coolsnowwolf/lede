/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

#include "rt_config.h"



UCHAR RateSwitchTable11B[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x04, 0x03,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 35, 45,
    0x03, 0x00,  3, 20, 45,
};

UCHAR RateSwitchTable11BG[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0a, 0x00,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 35, 45,
    0x03, 0x00,  3, 20, 45,
    0x04, 0x10,  2, 20, 35,
    0x05, 0x10,  3, 16, 35,
    0x06, 0x10,  4, 10, 25,
    0x07, 0x10,  5, 16, 25,
    0x08, 0x10,  6, 10, 25,
    0x09, 0x10,  7, 10, 13,
};

UCHAR RateSwitchTable11G[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x08, 0x00,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x10,  0, 20, 101,
    0x01, 0x10,  1, 20, 35,
    0x02, 0x10,  2, 20, 35,
    0x03, 0x10,  3, 16, 35,
    0x04, 0x10,  4, 10, 25,
    0x05, 0x10,  5, 16, 25,
    0x06, 0x10,  6, 10, 25,
    0x07, 0x10,  7, 10, 13,
};


#ifdef DOT11_N_SUPPORT
UCHAR RateSwitchTable11N1S[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0c, 0x0a,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x21,  5, 10, 25,
    0x09, 0x21,  6,  8, 14,
    0x0a, 0x21,  7,  8, 14,
    0x0b, 0x23,  7,  8, 14,
};


UCHAR RateSwitchTable11N2S[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0e, 0x0c,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x20, 11, 15, 30,
    0x09, 0x20, 12, 15, 30,
    0x0a, 0x20, 13,  8, 20,
    0x0b, 0x20, 14,  8, 20,
    0x0c, 0x20, 15,  8, 25,
    0x0d, 0x22, 15,  8, 15,
};


UCHAR RateSwitchTable11N3S[] = {
/* Item No.	Mode	Curr-MCS	TrainUp	TrainDown	 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x11, 0x0c,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x20, 11, 15, 30,
    0x09, 0x20, 12, 15, 22,
    0x0a, 0x20, 13,  8, 20,
    0x0b, 0x20, 14,  8, 20,
    0x0c, 0x20, 20,  8, 20,
    0x0d, 0x20, 21,  8, 20,
    0x0e, 0x20, 22,  8, 20,
    0x0f, 0x20, 23,  8, 20,
    0x10, 0x22, 23,  8, 15,
};


UCHAR RateSwitchTable11BGN1S[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0c, 0x0a,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x21,  5, 10, 25,
    0x09, 0x21,  6,  8, 14,
    0x0a, 0x21,  7,  8, 14,
    0x0b, 0x23,  7,  8, 14,
};


UCHAR RateSwitchTable11BGN2S[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0e, 0x0c,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x20, 11, 15, 30,
    0x09, 0x20, 12, 15, 22,
    0x0a, 0x20, 13,  8, 20,
    0x0b, 0x20, 14,  8, 20,
    0x0c, 0x20, 15,  8, 20,
    0x0d, 0x22, 15,  8, 15,
};


UCHAR RateSwitchTable11BGN3S[] = { /* 3*3*/
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0e, 0x00,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x21,  0, 30,101,	/*50*/
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 20, 50,
    0x04, 0x21,  4, 15, 50,
    0x05, 0x20, 11, 15, 30,
    0x06, 0x20, 12, 15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    /*0x0a, 0x20, 20, 15, 30,*/
    0x0a, 0x20, 21,  8, 20,
    0x0b, 0x20, 22,  8, 20,
    0x0c, 0x20, 23,  8, 25,
    0x0d, 0x22, 23,  8, 25,
};


UCHAR RateSwitchTable11N1SForABand[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown	*/
/* Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF) */
    0x09, 0x07,  0,  0,  0,						/* Initial used item after association */
    0x00, 0x21,  0, 30, 101,
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 10, 25,
    0x06, 0x21,  6,  8, 14,
    0x07, 0x21,  7,  8, 14,
    0x08, 0x23,  7,  8, 14,
};


UCHAR RateSwitchTable11N2SForABand[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0b, 0x09,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x21,  0, 30, 101,
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 15, 30,
    0x06, 0x20, 12,  15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    0x0a, 0x22, 15,  8, 25,
};


UCHAR RateSwitchTable11BGN2SForABand[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0b, 0x09,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x21,  0, 30,101,	/*50*/
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 15, 30,
    0x06, 0x20, 12, 15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    0x0a, 0x22, 15,  8, 25,
};


UCHAR RateSwitchTable11N3SForABand[] = { /* 3*3*/
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0e, 0x09,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x21,  0, 30, 101,
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 15, 30,
    0x06, 0x20, 12,  15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    /*0x0a, 0x22, 15,  8, 25,*/
    /*0x0a, 0x20, 20, 15, 30,*/
    0x0a, 0x20, 21,  8, 20,
    0x0b, 0x20, 22,  8, 20,
    0x0c, 0x20, 23,  8, 25,
    0x0d, 0x22, 23,  8, 25,
};


UCHAR RateSwitchTable11BGN3SForABand[] = { /* 3*3*/
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0e, 0x09,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x21,  0, 30,101,	/*50*/
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 15, 30,
    0x06, 0x20, 12, 15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    /*0x0a, 0x22, 15,  8, 25,*/
    /*0x0a, 0x20, 20, 15, 30,*/
    0x0a, 0x20, 21,  8, 20,
    0x0b, 0x20, 22,  8, 20,
    0x0c, 0x20, 23,  8, 25,
    0x0d, 0x22, 23,  8, 25,
};


#ifdef NEW_RATE_ADAPT_SUPPORT


UCHAR RateSwitchTableAdapt11B[] = {
/*  item no.   mcs   TrainDown  upMcs3     upMcs1
           mode   TrainUp  downMcs     upMcs2
*/
	 4,    3,  0,  0,   0,   0,   0,    0,   0,  0,
	 0, 0x00,  0, 40, 101,   0,   1,    1,   1,  1, /* CCK 1M */
	 1, 0x00,  1, 40,  50,   0,   2,    2,   2,  2, /* CCK 2M */
	 2, 0x00,  2, 35,  45,   1,   3,    3,   3,  5, /* CCK 5M */
	 3, 0x00,  3, 20,  45,   2,   3,    3,   3, 11, /* CCK 11M*/
};

UCHAR RateSwitchTableAdapt11BG[] = {
/*  item no.   mcs   TrainDown  upMcs3     upMcs1
           mode   TrainUp  downMcs     upMcs2
*/
	 10,   9,  0,  0,   0,   0,    0,     0,   0,   0,
	 0, 0x00,  0, 40, 101,   0,    1,     1,   1,   1, /* CCK 1M */
	 1, 0x00,  1, 40,  50,   0,    2,     2,   2,   2, /* CCK 2M */
	 2, 0x00,  2, 35,  45,   1,    3,     3,   3,   5, /* CCK 5M */
	 3, 0x00,  3, 20,  45,   2,    4,     4,   4,  11, /* CCK 11M*/
	 4, 0x10,  2, 20,  35,   3,    5,     5,   5,  12, /* OFDM 12M */
	 5, 0x10,  3, 16,  35,   4,    6,     6,   6,  18, /* OFDM 18M */
	 6, 0x10,  4, 10,  25,   5,    7,     7,   7,  24, /* OFDM 24M */
	 7, 0x10,  5, 16,  25,   6,    8,     8,   8,  36, /* OFDM 36M */
	 8, 0x10,  6, 10,  25,   7,    9,     9,   9,  48, /* OFDM 48M */
	 9, 0x10,  7, 10,  13,   8,    9,     9,   9,  54, /* OFDM 54M */
};

UCHAR RateSwitchTableAdapt11G[] = {
/*  item no.   mcs   TrainDown  upMcs3     upMcs1
           mode   TrainUp  downMcs     upMcs2
*/
	 8,    7,  0,  0,    0,   0,    0,     0,   0,   0,
	 0, 0x10,  0,  20, 101,   0,    1,     1,   1,   6, /* OFDM 6M */
	 1, 0x10,  1,  20,  35,   0,    2,     2,   2,   9, /* OFDM 9M */
	 2, 0x10,  2,  20,  35,   1,    3,     3,   3,  12, /* OFDM 12M */
	 3, 0x10,  3,  16,  35,   2,    4,     4,   4,  18, /* OFDM 18M*/
	 4, 0x10,  4,  10,  25,   3,    5,     5,   5,  24, /* OFDM 24M */
	 5, 0x10,  5,  16,  25,   4,    6,     6,   6,  36, /* OFDM 36M */
	 6, 0x10,  6,  10,  25,   5,    7,     7,   7,  48, /* OFDM 48M */
	 7, 0x10,  7,  10,  13,   6,    7,     7,   7,  54, /* OFDM 54M */
};

#ifdef RANGE_EXTEND
#define SUPPORT_SHORT_GI_RA		/* Support switching to Short GI rates in RA */
#endif /*  RANGE_EXTEND */

#ifdef MULTI_CLIENT_SUPPORT
UCHAR RateSwitchTableAdapt11N1SForMultiClients[] = {
/*  item no.   mcs   highPERThrd  upMcs3     upMcs1
           mode   lowPERThrd  downMcs     upMcs2
*/
	 12,   7,  0,  0,   0,   0,    0,     0,   0,   0,
	 0, 0x21,  0, 30,  50,  11,    1,     1,   1,   7,/* mcs0 */
	 1, 0x21,  1, 20,  50,   0,    2,     2,   2,  13,/* mcs1 */
	 2, 0x21,  2, 20,  50,   1,    3,     3,   3,  20,/* mcs2 */
	 3, 0x21,  3, 15,  50,   2,    4,     4,   4,  26,/* mcs3 */
	 4, 0x21,  4, 15,  30,   3,    5,     5,   5,  39,/* mcs4 */
	 5, 0x21,  5, 15,  30,   4,    6,     6,   6,  52,/* mcs5 */
	 6, 0x21,  6,  15,  22,   5,    7,     7,   7,  59,/* mcs6 */
	 7, 0x21,  7,  15,  22,   6,    8,     8,   8,  65,/* mcs7 */
	 8, 0x23,  7,  12,  18,   7,    8,     8,   8,  72,/* mcs7+short gi */


	 9, 0x00,  0, 40,  101,  9 ,  10,    10,   10,  1, /* cck-1M */
	10, 0x00,  1, 40,  50,   9,   11,    11,   11,  2, /* cck-2M */
	11, 0x10,  0, 30,  50,  10,    0,     0,   0,   6, /* OFDM 6M */	
};

UCHAR RateSwitchTableAdapt11N2SForMultiClients[] = {
/* item no.   mcs   highPERThrd  upMcs3    upMcs1
        mode   lowPERThrd  downMcs    upMcs2
*/
	22,   15,  0,  0,   0,   0,    0,   0,   0,   0,
	 0, 0x21,  0, 30,  50,  21,    8,   8,   1,   7,/* mcs0 */
	 1, 0x21,  1, 20,  50,   0,    9,   9,   2,  13,/* mcs1 */
	 2, 0x21,  2, 20,  50,   1,    9,   9,   3,  20,/* mcs2 */
	 3, 0x21,  3, 15,  50,   2,   10,  10,   4,  26,/* mcs3 */
	 4, 0x21,  4, 15,  30,   3,   11,  11,   5,  39,/* mcs4 */

	5, 0x21,  5, 15,  30,   4,   12,  12,   6,  52,/* mcs5 */
	6, 0x21,  6,  15,  22,   5,   12,  12,   7,  59,/* mcs6 */
#ifdef SUPPORT_SHORT_GI_RA
	7, 0x21,  7,  12,  18,   6,   12,  12, sg07,  65,/* mcs7 */
#else
	7, 0x21,  7,  12,  18,   6,   12,  12,   7,  65,/* mcs7 */
#endif

	 8, 0x20,  8, 30,  50,   0,    9,   9,   2,  13,/* mcs8 */
	 9, 0x20,  9, 20,  50,   8,   10,  10,   4,  26,/* mcs9 */
	10, 0x20, 10, 20,  40,   9,   11,  11,   5,  39,/* mcs10 */
	11, 0x20, 11, 15,  30,  10,   12,  12,   6,  52,/* mcs11 */
	12, 0x20, 12, 15,  30,  11,   13,  13,  12,  78,/* mcs12 */
	13, 0x20, 13,  8,  20,  12,   14,  14,  13, 104,/* mcs13 */
#ifdef SUPPORT_SHORT_GI_RA
	14, 0x20, 14,  8,  18,  13,   15,  15,sg14, 117,/* mcs14 */
	15, 0x20, 15,  8,  25,  14, sg15,sg15,sg14, 130,/* mcs15 */
	16, 0x22, 15,  8,  25,  15, sg15,sg15,sg15, 144,/* mcs15+shortGI */

	17, 0x22, 14,  8,  14,  14, sg15,sg15,  15, 130, /* mcs14+shortGI */
	18, 0x23,  7,  8,  14,   7,   12,  12,sg07,  72, /* mcs7+shortGI */
#else
	14, 0x20, 14,  8,  18,  13,   15,  15,  14, 117,/* mcs14 */
	15, 0x20, 15,  8,  25,  14,   16,  16,  15, 130,/* mcs15 */
	16, 0x22, 15,  8,  25,  15,   16,  16,  16, 144,/* mcs15+shortGI */
  17,    0,  0,  0,   0,   0,   0,    0,   0,   0,
  18,    0,  0,  0,   0,   0,   0,    0,   0,   0,
#endif
  19, 0x00,  0, 40,  101, 19 ,  19,    19,   20,  1, /* cck-1M */
  20, 0x00,  1, 40,  50,  19,   20,    20,   21,  2, /* cck-2M */
    21, 0x10,  0, 30,  50,  20,   8,     8,    0,   6, /* OFDM 6M */
};
#endif /*end MULTI_CLIENT_SUPPORT */

#ifdef INTERFERENCE_RA_SUPPORT
/*
	Rate switch tables for New Rate Adaptation

	Each row has 10 bytes of data.
	First row contains table information:
		Byte0=the number of rate entries, Byte1=the initial rate.
	Format of Mode byte:
		Bit0: STBC,
		Bit1: Short GI,
		Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)
*/
UCHAR RateSwitchTableAdapt11N1SForInterference[] = {
/*  item no.   mcs   highPERThrd  upMcs3     upMcs1
           mode   lowPERThrd  downMcs     upMcs2
*/
	 12,   7,  0,  0,   0,   0,    0,     0,   0,   0,
	 0, 0x21,  0, 30,  50,  11,    1,     1,   1,   7,/* mcs0 */
	 1, 0x21,  1, 20,  50,   0,    2,     2,   2,  13,/* mcs1 */
	 2, 0x21,  2, 20,  50,   1,    3,     3,   3,  20,/* mcs2 */
	 3, 0x21,  3, 15,  50,   2,    4,     4,   4,  26,/* mcs3 */
	 4, 0x21,  4, 15,  30,   3,    5,     5,   5,  39,/* mcs4 */
	 5, 0x21,  5, 10, 101,   5,    6,     6,   6,  52,/* mcs5 */
	 6, 0x21,  6,  8,  14,   5,    7,     7,   7,  59,/* mcs6 */
	 7, 0x21,  7,  8,  14,   6,    8,     8,   8,  65,/* mcs7 */
	 8, 0x23,  7,  8,  14,   7,    8,     8,   8,  72,/* mcs7+short gi */

	 9, 0x00,  0, 40,  101,  9 ,  10,    10,   10,  1, /* cck-1M */
	10, 0x00,  1, 40,  50,   9,   11,    11,   11,  2, /* cck-2M */
	11, 0x10,  0, 30,  50,  10,    0,     0,   0,   6, /* OFDM 6M */	
};

UCHAR RateSwitchTableAdapt11N2SForInterference[] = {
/* item no.   mcs   highPERThrd  upMcs3    upMcs1
        mode   lowPERThrd  downMcs    upMcs2
*/
	22,   15,  0,  0,   0,   0,    0,   0,   0,   0,
	 0, 0x21,  0, 30,  50,  21,    8,   8,   1,   7,/* mcs0 */
	 1, 0x21,  1, 20,  50,   0,    9,   9,   2,  13,/* mcs1 */
	 2, 0x21,  2, 20,  50,   1,    9,   9,   3,  20,/* mcs2 */
	 3, 0x21,  3, 15,  50,   2,   10,  10,   4,  26,/* mcs3 */
	 4, 0x21,  4, 15,  30,   3,   11,  11,   5,  39,/* mcs4 */
	 5, 0x21,  5, 10,  25,   4,   12,  12,   6,  52,/* mcs5 */
	 6, 0x21,  6,  8,  14,   5,   12,  12,   7,  59,/* mcs6 */
#ifdef SUPPORT_SHORT_GI_RA
	 7, 0x21,  7,  8,  14,   6,   12,  12, sg07,  65,/* mcs7 */
#else
	 7, 0x21,  7,  8,  14,   6,   12,  12,   7,  65,/* mcs7 */
#endif
	 8, 0x20,  8, 30,  50,   0,    9,   9,   2,  13,/* mcs8 */
	 9, 0x20,  9, 20,  50,   8,   10,  10,   4,  26,/* mcs9 */
	10, 0x20, 10, 20,  40,   9,   11,  11,   5,  39,/* mcs10 */
	11, 0x20, 11, 15,  30,  10,   12,  12,   6,  52,/* mcs11 */
	12, 0x20, 12, 15, 101,  11,   13,  13,  12,  78,/* mcs12 */
	13, 0x20, 13,  8,  20,  12,   14,  14,  13, 104,/* mcs13 */
#ifdef SUPPORT_SHORT_GI_RA
	14, 0x20, 14,  8,  18,  13,   15,  15,sg14, 117,/* mcs14 */
	15, 0x20, 15,  8,  25,  14, sg15,sg15,sg14, 130,/* mcs15 */
	16, 0x22, 15,  8,  25,  15, sg15,sg15,sg15, 144,/* mcs15+shortGI */

	17, 0x22, 14,  8,  14,  14, sg15,sg15,  15, 130, /* mcs14+shortGI */
	18, 0x23,  7,  8,  14,   7,   12,  12,sg07,  72, /* mcs7+shortGI */
#else
	14, 0x20, 14,  8,  18,  13,   15,  15,  14, 117,/* mcs14 */
	15, 0x20, 15,  8,  25,  14,   16,  16,  15, 130,/* mcs15 */
	16, 0x22, 15,  8,  25,  15,   16,  16,  16, 144,/* mcs15+shortGI */
  17,    0,  0,  0,   0,   0,   0,    0,   0,   0,
  18,    0,  0,  0,   0,   0,   0,    0,   0,   0,
#endif
  19, 0x00,  0, 40,  101, 19 ,  19,    19,   20,  1, /* cck-1M */
  20, 0x00,  1, 40,  50,  19,   20,    20,   21,  2, /* cck-2M */
    21, 0x10,  0, 30,  50,  20,   8,     8,    0,   6, /* OFDM 6M */
};

#endif /* end INTERFERENCE_RA_SUPPORT */
/*
	Rate switch tables for New Rate Adaptation

	Each row has 10 bytes of data.
	First row contains table information:
		Byte0=the number of rate entries, Byte1=the initial rate.
	Format of Mode byte:
		Bit0: STBC,
		Bit1: Short GI,
		Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)
*/
UCHAR RateSwitchTableAdapt11N1S[] = {
/*  item no.   mcs   highPERThrd  upMcs3     upMcs1
           mode   lowPERThrd  downMcs     upMcs2
*/
	 12,   7,  0,  0,   0,   0,    0,     0,   0,   0,
	 0, 0x21,  0, 30,  50,  11,    1,     1,   1,   7,/* mcs0 */
	 1, 0x21,  1, 20,  50,   0,    2,     2,   2,  13,/* mcs1 */
	 2, 0x21,  2, 20,  50,   1,    3,     3,   3,  20,/* mcs2 */
	 3, 0x21,  3, 15,  50,   2,    4,     4,   4,  26,/* mcs3 */
	 4, 0x21,  4, 15,  30,   3,    5,     5,   5,  39,/* mcs4 */
	 5, 0x21,  5, 10,  25,   4,    6,     6,   6,  52,/* mcs5 */
	 6, 0x21,  6,  8,  14,   5,    7,     7,   7,  59,/* mcs6 */
	 7, 0x21,  7,  8,  14,   6,    8,     8,   8,  65,/* mcs7 */
	 8, 0x23,  7,  8,  14,   7,    8,     8,   8,  72,/* mcs7+short gi */

	 9, 0x00,  0, 40,  101,  9,  11,    11,   11,  1, /* cck-1M */
	10, 0x00,  1, 40,  50,   9,   11,    11,   11,  2, /* cck-2M */
	11, 0x10,  0, 30,  50,  10,    0,     0,   0,   6, /* OFDM 6M */
};

#ifdef SUPPORT_SHORT_GI_RA
/*  Indices for Short GI rates in 11N2S table */
  #define sg07	18		/*  mcs7+shortGI index */
  #define sg14	17		/*  mcs14+shortGI index */
  #define sg15	16		/*  mcs15+shortGI index */
#endif

UCHAR RateSwitchTableAdapt11N2S[] = {
/* item no.   mcs   highPERThrd  upMcs3    upMcs1
        mode   lowPERThrd  downMcs    upMcs2
*/
	22,   15,  0,  0,   0,   0,    0,   0,   0,   0,
	 0, 0x21,  0, 30,  50,  21,    8,   8,   1,   7,/* mcs0 */
	 1, 0x21,  1, 20,  50,   0,    9,   9,   2,  13,/* mcs1 */
	 2, 0x21,  2, 20,  50,   1,    9,   9,   3,  20,/* mcs2 */
	 3, 0x21,  3, 15,  50,   2,   10,  10,   4,  26,/* mcs3 */
	 4, 0x21,  4, 15,  30,   3,   11,  11,   5,  39,/* mcs4 */
	 5, 0x21,  5, 10,  25,   4,   12,  12,   6,  52,/* mcs5 */
	 6, 0x21,  6,  8,  14,   5,   12,  12,   7,  59,/* mcs6 */
#ifdef SUPPORT_SHORT_GI_RA
	 7, 0x21,  7,  8,  14,   6,   12,  12, sg07,  65,/* mcs7 */
#else
	 7, 0x21,  7,  8,  14,   6,   12,  12,   7,  65,/* mcs7 */
#endif
	 8, 0x20,  8, 30,  50,   0,    9,   9,   2,  13,/* mcs8 */
	 9, 0x20,  9, 20,  50,   8,   10,  10,   4,  26,/* mcs9 */
	10, 0x20, 10, 20,  40,   9,   11,  11,   5,  39,/* mcs10 */
	11, 0x20, 11, 15,  30,  10,   12,  12,   6,  52,/* mcs11 */
	12, 0x20, 12, 15,  30,  11,   13,  13,  12,  78,/* mcs12 */
	13, 0x20, 13,  8,  20,  12,   14,  14,  13, 104,/* mcs13 */
#ifdef SUPPORT_SHORT_GI_RA
	14, 0x20, 14,  8,  18,  13,   15,  15,sg14, 117,/* mcs14 */
	15, 0x20, 15,  8,  25,  14, sg15,sg15,sg14, 130,/* mcs15 */
	16, 0x22, 15,  8,  25,  15, sg15,sg15,sg15, 144,/* mcs15+shortGI */

	17, 0x22, 14,  8,  14,  14, sg15,sg15,  15, 130, /* mcs14+shortGI */
	18, 0x23,  7,  8,  14,   7,   12,  12,sg07,  72, /* mcs7+shortGI */
#else
	14, 0x20, 14,  8,  18,  13,   15,  15,  14, 117,/* mcs14 */
	15, 0x20, 15,  8,  25,  14,   16,  16,  15, 130,/* mcs15 */
	16, 0x22, 15,  8,  25,  15,   16,  16,  16, 144,/* mcs15+shortGI */
	17,    0,  0,  0,   0,   0,   0,    0,   0,   0,
	18,    0,  0,  0,   0,   0,   0,    0,   0,   0,
#endif
	19, 0x00,  0, 40,  101, 19,  19,    19,   21,  1, /* cck-1M */
	20, 0x00,  1, 40,  50,  19,   20,    20,   21,  2, /* cck-2M */
    21, 0x10,  0, 30,  50,  20,   8,     8,    0,   6, /* OFDM 6M */
};

#ifdef SUPPORT_SHORT_GI_RA
/*  Indices for Short GI rates in 11N3S table */
  #undef sg07
  #undef sg14
  #undef sg15
  #define sg07	29		/*  mcs7+shortGI index */
  #define sg14	27		/*  mcs14+shortGI index */
  #define sg15	28		/*  mcs15+shortGI index */
  #define sg21	25		/*  mcs21+shortGI index */
  #define sg22	26		/*  mcs22+shortGI index */
  #define sg23	24		/*  mcs23+shortGI index */
#endif

UCHAR RateSwitchTableAdapt11N3S[] = {
/* item no   mcs   highPERThrd   upMcs3     upMcs1
        mode   lowPERThrd  downMcs     upMcs2
*/
	33,   23,  0,  0,   0,   0,    0,    0,    0,   0,
	 0, 0x21,  0, 30,  50,  32,    1,    8,    1,   7, /* mcs0 */
     1, 0x21,  1, 20,  50,   0,   16,    9,    2,  13, /* mcs1 */
     2, 0x21,  2, 20,  50,   1,   17,    9,    3,  20, /* mcs2 */
     3, 0x21,  3, 15,  50,   2,   17,   10,    4,  26, /* mcs3 */
     4, 0x21,  4, 15,  30,   3,   18,   11,    5,  39, /* mcs4 */
     5, 0x21,  5, 10,  25,   4,   18,   12,    6,  52, /* mcs5 */
     6, 0x21,  6,  8,  14,   5,   19,   12,    7,  59, /* mcs6 */
#ifdef SUPPORT_SHORT_GI_RA
	 7, 0x21,  7,  8,  14,   6,   19,   12, sg07,  65, /* mcs7 */
#else
	 7, 0x21,  7,  8,  14,   6,   19,   12,    7,  65, /* mcs7 */
#endif
     8, 0x20,  8, 30,  50,   0,   16,    9,    2,  13, /* mcs8 */
     9, 0x20,  9, 20,  50,   8,   17,   10,    4,  26, /* mcs9 */
    10, 0x20, 10, 20,  40,   9,   18,   11,    5,  39, /* mcs10 */
    11, 0x20, 11, 15,  30,  10,   18,   12,    6,  52, /* mcs11 */
    12, 0x20, 12, 15,  30,  11,   20,   13,   12,  78, /* mcs12 */
    13, 0x20, 13,  8,  20,  12,   20,   14,   13, 104, /* mcs13 */
#ifdef SUPPORT_SHORT_GI_RA
    14, 0x20, 14,  8,  18,  13,   21,   15, sg14, 117, /* mcs14 */
    15, 0x20, 15,  8,  14,  14,   21, sg15, sg14, 130, /* mcs15 */
#else
    14, 0x20, 14,  8,  18,  13,   21,   15,   14, 117, /* mcs14 */
    15, 0x20, 15,  8,  14,  14,   21,   15,   15, 130, /* mcs15 */
#endif
    16, 0x20, 16, 30,  50,   8,   17,    9,    3,  20, /* mcs16 */
    17, 0x20, 17, 20,  50,  16,   18,   11,    5,  39, /* mcs17 */
    18, 0x20, 18, 20,  40,  17,   19,   12,    7,  59, /* mcs18 */
    19, 0x20, 19, 15,  30,  18,   20,   13,   19,  78, /* mcs19 */
    20, 0x20, 20, 15,  30,  19,   21,   15,   20, 117, /* mcs20 */
#ifdef SUPPORT_SHORT_GI_RA
    21, 0x20, 21,  8,  20,  20,   22, sg21,   21, 156, /* mcs21 */
    22, 0x20, 22,  8,  20,  21,   23, sg22, sg21, 176, /* mcs22 */
    23, 0x20, 23,  6,  18,  22, sg23,   23, sg22, 195, /* mcs23 */
    24, 0x22, 23,  6,  14,  23, sg23, sg23, sg23, 217, /* mcs23+shortGI */

    25, 0x22, 21,  6,  18,  21, sg22,   22, sg21, 173, /* mcs21+shortGI */
    26, 0x22, 22,  6,  18,  22, sg23,   23, sg22, 195, /* mcs22+shortGI */
    27, 0x22, 14,  8,  14,  14,   21, sg15,   15, 130, /* mcs14+shortGI */
    28, 0x22, 15,  8,  14,  15,   21, sg15, sg15, 144, /* mcs15+shortGI */
    29, 0x23,  7,  8,  14,   7,   19,   12,   29,  72, /* mcs7+shortGI */
#else
    21, 0x20, 21,  8,  20,  20,   22,   21,   21, 156, /* mcs21 */
    22, 0x20, 22,  8,  20,  21,   23,   22,   22, 176, /* mcs22 */
    23, 0x20, 23,  6,  18,  22,   24,   23,   23, 195, /* mcs23 */
    24, 0x22, 23,  6,  14,  23,   24,   24,   24, 217, /* mcs23+shortGI */
    25,    0,  0,  0,   0,   0,   0,     0,    0,   0,
    26,    0,  0,  0,   0,   0,   0,     0,    0,   0,
    27,    0,  0,  0,   0,   0,   0,     0,    0,   0,
    28,    0,  0,  0,   0,   0,   0,     0,    0,   0,
    29,    0,  0,  0,   0,   0,   0,     0,    0,   0,
#endif
    30, 0x00,  0, 40,  101, 30 ,  30,    30,   31,  1, /* cck-1M */
    31, 0x00,  1, 40,  50,  30,   31,    31,   32,  2, /* cck-2M */
    32, 0x21, 32, 30,  50,  31,   0,     8,    0,   7, /* mcs32 or 20M/mcs0 */
};


#ifdef DOT11_VHT_AC
/*
	VHT-capable rate table
	
	Each row has 10 bytes of data.

	1. First row contains table info, which is initial used item after assoc:
			Byte0=the number of rate entries, Byte1=the initial rate.

	2. rest rows Format:
	[Index] [Mode] [nSS] [CurrMCS] [PERThrd Low/High] [downMCS] [upMCS3/2/1]

	[Mode]: 
		bit0: STBC
		bit1: Short GI
		bit2~3: BW - (20M/40M/80M)
		bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
		bit7: Reserved
	[nSS]:
		Number of Spatial Stream

	Note: downMCS, upMCS3, upMCS2 and upMCS1 are zero-based array index.	
*/
/* 1x1 VHT-capable rate table */
UCHAR RateTableVht1S[] = 
{
	11,		9,		0,		0, 0,				0,		0, 0, 0,		0,
/*	[Idx]	[Mode]	[MCS]	[PER_Low/High]	[dMCS]	[upMCS3/2/1] [nSS]*/
	0,		0x10,	0, 		30, 101,		0,		0, 0, 1,		1,
	1,		0x25,	0, 		30, 50,			0,		0, 0, 2,		1,
	2,		0x49,	0,		30, 50,			1,		0, 0, 3,		1,
	3,		0x49,	1,		20, 50,			2,		0, 0, 4,		1, 
	4,		0x49,	2,		20, 50,			3,		0, 0, 5, 		1,
	5,		0x49,	3,		15, 50,			4,		0, 0, 6, 		1,
	6,		0x49,	4,		15, 30,			5,		0, 0, 7, 		1,
	7,		0x49,	5,		10, 25,			6,		0, 0, 8, 		1,
	8,		0x49,	6,		8, 14,			7,		0, 0, 9, 		1,
	9,		0x49,	7,		8, 14,			8,		0, 0, 10, 		1,
	10,		0x4B,	7,		8, 14,			9,		0, 0, 10, 		1,
};


UCHAR RateTableVht1S_MCS9[] = 
{
	14,		12,			0,		0, 0,				0,		0, 0, 0,		0,
/*	[Idx]	[Mode]		[MCS]	[PER_Low/High]	[dMCS]	[upMCS3/2/1] [nSS] */
	0,		0x10,		0, 		30, 101,			0,		0, 0, 1,		1, /* OFDM, MCS0, BW20 */
	1,		0x45,		0, 		30, 50,				0,		0, 0, 2,		1, /* VHT, MCS0, BW40, STBC */
	2,		0x49,		0,		30, 50,				1,		0, 0, 3,		1, /* VHT, MCS0, BW80, STBG */
	3,		0x49,		1,		20, 50,				2,		0, 0, 4,		1, /* VHT, MCS1, BW80, STBG */
	4,		0x49,		2,		20, 50,				3,		0, 0, 5, 		1, /* VHT, MCS2, BW80, STBG */
	5,		0x49,		3,		15, 50,				4,		0, 0, 6, 		1, /* VHT, MCS3, BW80, STBG */
	6,		0x49,		4,		15, 30,				5,		0, 0, 7, 		1, /* VHT, MCS4, BW80, STBG */
	7,		0x49,		5,		10, 25,				6,		0, 0, 8, 		1, /* VHT, MCS5, BW80, STBG */
	8,		0x49,		6,		8, 14,				7,		0, 0, 9, 		1, /* VHT, MCS6, BW80, STBG */
	9,		0x49,		7,		8, 14,				8,		0, 0, 10, 		1, /* VHT, MCS7, BW80, STBG */
	10,		0x4B,		7,		8, 14,				9,		0, 0, 11, 		1, /* VHT, MCS7, BW80, STBG, SGI */
	11,		0x49,		8,		8, 14,				10,		0, 0, 12, 		1, /* VHT, MCS8, BW80, STBG */
	12,		0x49,		9,		8, 14,				11,		0, 0, 13, 		1, /* VHT, MCS9, BW80, STBG */
	13,		0x4A,		9,		8, 14,				12,		0, 0, 13, 		1, /* VHT, MCS9, BW80, SGI */
};

/* 2x2 VHT-capable rate table */
// TODO: shiang-usw, refine this!!
UCHAR RateTableVht2S_old[] = 
{
	22,		21,		0,		0, 0,				0,		0, 0,0,		0,
/*	[Idx]	[Mode]	[MCS]	[PER_Low/High]	[dMCS]	[upMCS3/2/1] [nSS]*/
	0,		0x10,	0,		30, 101,		0,		0, 1, 1,		1, /* OFDM 1x1 MCS0 BW20 */
	1,		0x25,	0,		30, 50,			0,		0, 2, 2,		1, /* HT 1x1 MCS0 BW40 */
	2,		0x49,	0,		30, 50,			1,		0, 10, 3,		1, /* VHT 1x1 MCS0 BW80 LGI */
	3,		0x49,	1,		20, 50,			2,		0, 11, 4,		1, /* VHT 1x1 MCS1 BW80 LGI */ 
	4,		0x49,	2,		20, 50,			3,		0, 11, 5,		1, /* VHT 1x1 MCS2 BW80 LGI */
	5,		0x49,	3,		15, 50,			4,		0, 12, 6,		1, /* VHT 1x1 MCS3 BW80 LGI */
	6,		0x49,	4,		15, 30,			5,		0, 13, 7,		1, /* VHT 1x1 MCS4 BW80 LGI */
	7,		0x49,	5,		10, 25,			6,		0, 14, 8,		1, /* VHT 1x1 MCS5 BW80 LGI */
	8,		0x49,	6,		8, 14,			7,		0, 14, 9,		1, /* VHT 1x1 MCS6 BW80 LGI */
	9,		0x49,	7,		8, 14,			8,		0, 14, 9,		1, /* VHT 1x1 MCS7 BW80 LGI */
	10,		0x48,	0,		30, 50,			2,		0, 11, 4,		2, /* VHT 2x2 MCS0 BW80 LGI */
	11,		0x48,	1,		20, 50,			10,		0, 12, 6,		2, /* VHT 2x2 MCS1 BW80 LGI */
	12,		0x48,	2,		20, 50,			11,		0, 13, 7,		2, /* VHT 2x2 MCS2 BW80 LGI */
	13,		0x48,	3,		15, 30,			12,		0, 14, 8,		2, /* VHT 2x2 MCS3 BW80 LGI */
	14,		0x48,	4,		15, 30,			13,		0, 15, 14,		2, /* VHT 2x2 MCS4 BW80 LGI */
	15,		0x48,	5,		8, 20,			14,		0, 16, 15, 		2, /* VHT 2x2 MCS5 BW80 LGI */	 
	16,		0x48,	6,		8, 18,			15,		0, 17, 16, 		2, /* VHT 2x2 MCS6 BW80 LGI */
	17,		0x48,	7,		8, 25,			16,		0, 18, 17, 		2, /* VHT 2x2 MCS7 BW80 LGI */
	18,		0x4A,	7,		8, 25,			17,		0, 19, 18, 		2, /* VHT 2x2 MCS7 BW80 SGI */
	19,		0x48,	8,		8, 25,			18,		0, 20, 19, 		2, /* VHT 2x2 MCS8 BW80 LGI */
	20, 	0x48,	9,		8, 25,			19,		0, 21, 20,		2, /* VHT 2x2 MCS9 BW80 LGI */
	21,		0x4A, 	9,		8, 25,			20,		0, 21, 21,  	2, /* VHT 2x2 MCS9 BW80 SGI */ 
};

UCHAR RateTableVht2S_MCS7[] =
{
  31,   30,   0,      0,   0,    0,   0,  0,  0,  0,
/*  [Idx] [Mode]  [MCS] [PER_down/up] [dMCS]  [upMCS3/2/1] [nSS] */
  /* reserved */
   0,   0x11, 0,    100, 101,    0,   0,  1,  1,  1,
   1,   0x11, 0,    100, 101,    0,   0,  2,  2,  1,
   2,   0x11, 0,    100, 101,    0,   0,  3,  3,  1,
  
  /* The Data Rate of OFDM for long distance */
   3,   0x11, 0,     20, 101,    3,   0,  4,  4,  1,/* OFDM 1x1 MCS0 BW20 */
   4,   0x11, 1,     20,  40,    3,   0,  5,  5,  1,/* OFDM 1x1 MCS1 BW20 */
   5,   0x11, 2,     20,  35,    4,   0,  6,  6,  1,/* OFDM 1x1 MCS2 BW20 */
   6,   0x11, 3,     15,  35,    5,   0,  7,  7,  1,/* OFDM 1x1 MCS3 BW20 */

  /* The Data Rate of VHT BW 20/40 */
   7,   0x41, 0,     15,  50,    6,   0,  8,  8,  1,/* VHT 1x1 MCS0 BW20 */
   8,   0x41, 1,     15,  50,    7,   0,  9,  9,  1,/* VHT 1x1 MCS1 BW20 */
   9,   0x45, 1,     15,  30,    8,   0, 10, 10,  1,/* VHT 1x1 MCS1 BW40 */
  
  /* The Group Data Rate of 1SS with VHT BW80 */
  10,   0x49, 0,     15,  25,    9,   0, 20, 11,  1,/* VHT 1x1 MCS0 BW80 LGI */
  11,   0x49, 1,     20,  35,   10,   0, 21, 12,  1,/* VHT 1x1 MCS1 BW80 LGI */
  12,   0x49, 2,     20,  35,   11,   0, 21, 13,  1,/* VHT 1x1 MCS2 BW80 LGI */
  13,   0x49, 3,     15,  30,   12,   0, 22, 14,  1,/* VHT 1x1 MCS3 BW80 LGI */
  14,   0x49, 4,     15,  30,   13,   0, 23, 15,  1,/* VHT 1x1 MCS4 BW80 LGI */
  15,   0x49, 5,     10,  25,   14,   0, 24, 16,  1,/* VHT 1x1 MCS5 BW80 LGI */
  16,   0x49, 6,      8,  15,   15,   0, 24, 17,  1,/* VHT 1x1 MCS6 BW80 LGI */
  17,   0x49, 7,      8,  15,   16,   0, 24, 24,  1,/* VHT 1x1 MCS7 BW80 LGI */
  18,   0x49, 8,      8,  15,   16,   0, 24, 24,  1,/* VHT 1x1 MCS8 BW80 LGI */
  19,   0x49, 9,      8,  15,   16,   0, 24, 24,  1,/* VHT 1x1 MCS9 BW80 LGI */

  /* The Group Data Rate of 2SS with VHT BW80 */
  20,   0x48, 0,     30,  50,   10,   0, 21, 12,  2,/* VHT 2x2 MCS0 BW80 LGI */
  21,   0x48, 1,     30,  50,   20,   0, 22, 14,  2,/* VHT 2x2 MCS1 BW80 LGI */
  22,   0x48, 2,     15,  30,   21,   0, 23, 15,  2,/* VHT 2x2 MCS2 BW80 LGI */
  23,   0x48, 3,     15,  30,   22,   0, 24, 16,  2,/* VHT 2x2 MCS3 BW80 LGI */
  24,   0x48, 4,     15,  30,   23,   0, 25, 25,  2,/* VHT 2x2 MCS4 BW80 LGI */
  25,   0x48, 5,      8,  20,   24,   0, 26, 26,  2,/* VHT 2x2 MCS5 BW80 LGI */
  26,   0x48, 6,      8,  20,   25,   0, 27, 27,  2,/* VHT 2x2 MCS6 BW80 LGI */
  27,   0x48, 7,      8,  20,   26,   0, 28, 28,  2,/* VHT 2x2 MCS7 BW80 LGI */
  28,   0x4A, 7,      5,  15,   27,   0, 28, 28,  2,/* VHT 2x2 MCS7 BW80 SGI */
  29,   0x48, 9,      5,  15,   28,   0, 28, 28,  2,/* VHT 2x2 MCS9 BW80 LGI */
  30,   0x4A, 9,      5,  15,   28,   0, 28, 28,  2,/* VHT 2x2 MCS9 BW80 SGI */
};


UCHAR RateTableVht2S[] = 

{

  31,   30,   0,      0,   0,    0,   0,  0,  0,  0,
/*  [Idx] [Mode]  [MCS] [PER_down/up] [dMCS]  [upMCS3/2/1] [nSS] */

  /* reserved */
  0,    0x11, 0,    100, 101,    0,   0,  1,  1,  1,
  1,    0x11, 0,    100, 101,    0,   0,  2,  2,  1,
  2,    0x11, 0,    100, 101,    1,   0,  3,  3,  1,

  /* The Data Rate of OFDM for long distance */
  3,    0x11, 0,     20, 101,    2,   0,  4,  4,  1,/* OFDM 1x1 MCS0 BW20 */
  4,    0x11, 1,     20,  40,    3,   0,  5,  5,  1,/* OFDM 1x1 MCS1 BW20 */
  5,    0x11, 2,     20,  35,    4,   0,  6,  6,  1,/* OFDM 1x1 MCS2 BW20 */
  6,    0x11, 3,     15,  35,    5,   0,  7,  7,  1,/* OFDM 1x1 MCS3 BW20 */

  /* The Data Rate of VHT BW 20/40 */
  7,    0x41, 0,     15,  50,    6,   0,  8,  8,  1,/* VHT 1x1 MCS0 BW20 */
  8,    0x41, 1,     15,  50,    7,   0,  9,  9,  1,/* VHT 1x1 MCS1 BW20 */
  9,    0x45, 1,     15,  30,    8,   0, 10, 10,  1,/* VHT 1x1 MCS1 BW40 */

  /* The Group Data Rate of 1SS with VHT BW80 */
  10,   0x49, 0,     15,  25,    9,   0, 20, 11,  1,/* VHT 1x1 MCS0 BW80 LGI */
  11,   0x49, 1,     20,  35,   10,   0, 21, 12,  1,/* VHT 1x1 MCS1 BW80 LGI */
  12,   0x49, 2,     20,  35,   11,   0, 21, 13,  1,/* VHT 1x1 MCS2 BW80 LGI */
  13,   0x49, 3,     15,  30,   12,   0, 22, 14,  1,/* VHT 1x1 MCS3 BW80 LGI */
  14,   0x49, 4,     15,  30,   13,   0, 23, 15,  1,/* VHT 1x1 MCS4 BW80 LGI */
  15,   0x49, 5,     10,  25,   14,   0, 24, 16,  1,/* VHT 1x1 MCS5 BW80 LGI */
  16,   0x49, 6,      8,  15,   15,   0, 24, 17,  1,/* VHT 1x1 MCS6 BW80 LGI */
  17,   0x49, 7,      8,  15,   16,   0, 24, 18,  1,/* VHT 1x1 MCS7 BW80 LGI */
  18,   0x49, 8,      8,  15,   17,   0, 25, 19,  1,/* VHT 1x1 MCS8 BW80 LGI */
  19,   0x49, 9,      8,  15,   18,   0, 25, 25,  1,/* VHT 1x1 MCS9 BW80 LGI */

  /* The Group Data Rate of 2SS with VHT BW 80 */
  20,   0x48, 0,     30,  50,   10,   0, 21, 12,  2,/* VHT 2x2 MCS0 BW80 LGI */
  21,   0x48, 1,     30,  50,   20,   0, 22, 14,  2,/* VHT 2x2 MCS1 BW80 LGI */
  22,   0x48, 2,     15,  30,   21,   0, 23, 15,  2,/* VHT 2x2 MCS2 BW80 LGI */
  23,   0x48, 3,     15,  30,   22,   0, 24, 16,  2,/* VHT 2x2 MCS3 BW80 LGI */
  24,   0x48, 4,     15,  30,   23,   0, 25, 19,  2,/* VHT 2x2 MCS4 BW80 LGI */
  25,   0x48, 5,      8,  20,   24,   0, 26, 26,  2,/* VHT 2x2 MCS5 BW80 LGI */
  26,   0x48, 6,      8,  20,   25,   0, 27, 27,  2,/* VHT 2x2 MCS6 BW80 LGI */
  27,   0x48, 7,      8,  20,   26,   0, 28, 28,  2,/* VHT 2x2 MCS7 BW80 LGI */
  28,   0x48, 8,      5,  15,   27,   0, 29, 29,  2,/* VHT 2x2 MCS8 BW80 LGI */
  29,   0x48, 9,      5,  15,   28,   0, 30, 30,  2,/* VHT 2x2 MCS9 BW80 LGI */
  30,   0x4A, 9,      5,  15,   29,   0, 30, 30,  2,/* VHT 2x2 MCS9 BW80 SGI */
};

UCHAR RateTableVht2S_BW40[] = 

{
  31,   30,   0,      0,   0,    0,   0,  0,  0,  0,
/*  [Idx] [Mode]  [MCS] [PER_down/up] [dMCS]  [upMCS3/2/1] [nSS] */
  /* reserved */
  0,    0x11, 0,    100, 101,    0,   0,  1,  1,  1,
  1,    0x11, 0,    100, 101,    0,   0,  2,  2,  1,
  2,    0x11, 0,    100, 101,    1,   0,  3,  3,  1,

  /* The Data Rate of OFDM for long distance */
  3,    0x11, 0,     20, 101,    2,   0,  4,  4,  1,/* OFDM 1x1 MCS0 BW20 */
  4,    0x11, 1,     20,  40,    3,   0,  5,  5,  1,/* OFDM 1x1 MCS1 BW20 */
  5,    0x11, 2,     20,  35,    4,   0,  6,  6,  1,/* OFDM 1x1 MCS2 BW20 */
  6,    0x11, 3,     15,  35,    5,   0,  7,  7,  1,/* OFDM 1x1 MCS3 BW20 */

  /* The Data Rate of VHT BW 20/40 */
  7,    0x41, 0,     15,  50,    6,   0,  8,  8,  1,/* VHT 1x1 MCS0 BW20 */
  8,    0x41, 1,     15,  50,    7,   0, 10, 10,  1,/* VHT 1x1 MCS1 BW20 */
  9,    0x00, 0,      0,   0,    0,   0,  0,  0,  0,

  /* The Group Data Rate of 1SS with VHT BW40 */
  10,   0x45, 0,     15,  25,    8,   0, 20, 11,  1,/* VHT 1x1 MCS0 BW40 LGI */
  11,   0x45, 1,     20,  35,   10,   0, 21, 12,  1,/* VHT 1x1 MCS1 BW40 LGI */
  12,   0x45, 2,     20,  35,   11,   0, 21, 13,  1,/* VHT 1x1 MCS2 BW40 LGI */
  13,   0x45, 3,     15,  30,   12,   0, 22, 14,  1,/* VHT 1x1 MCS3 BW40 LGI */
  14,   0x45, 4,     15,  30,   13,   0, 23, 15,  1,/* VHT 1x1 MCS4 BW40 LGI */
  15,   0x45, 5,     10,  25,   14,   0, 24, 16,  1,/* VHT 1x1 MCS5 BW40 LGI */
  16,   0x45, 6,      8,  15,   15,   0, 24, 17,  1,/* VHT 1x1 MCS6 BW40 LGI */
  17,   0x45, 7,      8,  15,   16,   0, 24, 18,  1,/* VHT 1x1 MCS7 BW40 LGI */
  18,   0x45, 8,      8,  15,   17,   0, 25, 19,  1,/* VHT 1x1 MCS8 BW40 LGI */
  19,   0x45, 9,      8,  15,   18,   0, 25, 25,  1,/* VHT 1x1 MCS9 BW40 LGI */

  /* The Group Data Rate of 2SS with VHT BW40 */

  20,   0x44, 0,     30,  50,   10,   0, 21, 12,  2,/* VHT 2x2 MCS0 BW40 LGI */
  21,   0x44, 1,     30,  50,   20,   0, 22, 14,  2,/* VHT 2x2 MCS1 BW40 LGI */
  22,   0x44, 2,     15,  30,   21,   0, 23, 15,  2,/* VHT 2x2 MCS2 BW40 LGI */
  23,   0x44, 3,     15,  30,   22,   0, 24, 16,  2,/* VHT 2x2 MCS3 BW40 LGI */
  24,   0x44, 4,     15,  30,   23,   0, 25, 19,  2,/* VHT 2x2 MCS4 BW40 LGI */
  25,   0x44, 5,      8,  20,   24,   0, 26, 26,  2,/* VHT 2x2 MCS5 BW40 LGI */
  26,   0x44, 6,      8,  20,   25,   0, 27, 27,  2,/* VHT 2x2 MCS6 BW40 LGI */
  27,   0x44, 7,      8,  20,   26,   0, 28, 28,  2,/* VHT 2x2 MCS7 BW40 LGI */
  28,   0x44, 8,      5,  15,   27,   0, 29, 29,  2,/* VHT 2x2 MCS8 BW40 LGI */
  29,   0x44, 9,      5,  15,   28,   0, 30, 30,  2,/* VHT 2x2 MCS9 BW40 LGI */
  30,   0x46, 9,      5,  15,   29,   0, 30, 30,  2,/* VHT 2x2 MCS9 BW40 SGI */
};

UCHAR RateTableVht2S_BW20[] = 

{
  30,   28,   0,      0,   0,    0,   0,  0,  0,  0,
/*  [Idx] [Mode]  [MCS] [PER_down/up] [dMCS]  [upMCS3/2/1] [nSS] */
  /* reserved */
  0,    0x11, 0,    100, 101,    0,   0,  1,  1,  1,
  1,    0x11, 0,    100, 101,    0,   0,  2,  2,  1,
  2,    0x11, 0,    100, 101,    1,   0,  3,  3,  1,

  /* The Data Rate of OFDM for long distance */
  3,    0x11, 0,     20, 101,    2,   0,  4,  4,  1,/* OFDM 1x1 MCS0 BW20 */
  4,    0x11, 1,     20,  40,    3,   0,  5,  5,  1,/* OFDM 1x1 MCS1 BW20 */
  5,    0x11, 2,     20,  35,    4,   0,  6,  6,  1,/* OFDM 1x1 MCS2 BW20 */
  6,    0x11, 3,     15,  35,    5,   0, 10, 10,  1,/* OFDM 1x1 MCS3 BW20 */
  7,    0x00, 0,      0,   0,    0,   0,  0,  0,  0,
  8,    0x00, 0,      0,   0,    0,   0,  0,  0,  0,
  9,    0x00, 0,      0,   0,    0,   0,  0,  0,  0,

  /* The Group Data Rate of 1SS with VHT BW80 */
  10,   0x41, 0,     15,  25,    6,   0, 20, 11,  1,/* VHT 1x1 MCS0 BW20 LGI */
  11,   0x41, 1,     20,  35,   10,   0, 21, 12,  1,/* VHT 1x1 MCS1 BW20 LGI */
  12,   0x41, 2,     20,  35,   11,   0, 21, 13,  1,/* VHT 1x1 MCS2 BW20 LGI */
  13,   0x41, 3,     15,  30,   12,   0, 22, 14,  1,/* VHT 1x1 MCS3 BW20 LGI */
  14,   0x41, 4,     15,  30,   13,   0, 23, 15,  1,/* VHT 1x1 MCS4 BW20 LGI */
  15,   0x41, 5,     10,  25,   14,   0, 24, 16,  1,/* VHT 1x1 MCS5 BW20 LGI */
  16,   0x41, 6,      8,  15,   15,   0, 24, 17,  1,/* VHT 1x1 MCS6 BW20 LGI */
  17,   0x41, 7,      8,  15,   16,   0, 24, 18,  1,/* VHT 1x1 MCS7 BW20 LGI */
  18,   0x41, 8,      8,  15,   17,   0, 25, 25,  1,/* VHT 1x1 MCS8 BW20 LGI */
  19,   0x00, 0,      0,   0,    0,   0,  0,  0,  0,

  /* The Group Data Rate of 2SS with VHT BW 80 */
  20,   0x40, 0,     30,  50,   10,   0, 21, 12,  2,/* VHT 2x2 MCS0 BW20 LGI */
  21,   0x40, 1,     30,  50,   20,   0, 22, 14,  2,/* VHT 2x2 MCS1 BW20 LGI */
  22,   0x40, 2,     15,  30,   21,   0, 23, 15,  2,/* VHT 2x2 MCS2 BW20 LGI */
  23,   0x40, 3,     15,  30,   22,   0, 24, 16,  2,/* VHT 2x2 MCS3 BW20 LGI */
  24,   0x40, 4,     15,  30,   23,   0, 25, 25,  2,/* VHT 2x2 MCS4 BW20 LGI */
  25,   0x40, 5,      8,  20,   24,   0, 26, 26,  2,/* VHT 2x2 MCS5 BW20 LGI */
  26,   0x40, 6,      8,  20,   25,   0, 27, 27,  2,/* VHT 2x2 MCS6 BW20 LGI */
  27,   0x40, 7,      8,  20,   26,   0, 28, 29,  2,/* VHT 2x2 MCS7 BW20 LGI */
  28,   0x40, 8,      5,  15,   27,   0, 29, 29,  2,/* VHT 2x2 MCS8 BW20 LGI */
  29,   0x42, 8,      5,  15,   28,   0, 29, 29,  2,/* VHT 2x2 MCS8 BW20 SGI */
};


#endif /* DOT11_VHT_AC */

#endif /*  NEW_RATE_ADAPT_SUPPORT */

#endif /* DOT11_N_SUPPORT */


/* MlmeGetSupportedMcs - fills in the table of mcs with index into the pTable
		pAd - pointer to adapter
		pTable - pointer to the Rate Table. Assumed to be a table without mcsGroup values
		mcs - table of MCS index into the Rate Table. -1 => not supported
*/
VOID MlmeGetSupportedMcs(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	*pTable,
	OUT CHAR 	mcs[])
{
	CHAR	idx;
	RTMP_RA_LEGACY_TB *pCurrTxRate;

	for (idx = 0; idx < 24; idx++)
		mcs[idx] = -1;

	/*  check the existence and index of each needed MCS */
	for (idx=0; idx<RATE_TABLE_SIZE(pTable); idx++)
	{
		pCurrTxRate = PTX_RA_LEGACY_ENTRY(pTable, idx);

		/*  Rate Table may contain CCK and MCS rates. Give HT/Legacy priority over CCK */
		if (pCurrTxRate->CurrMCS==MCS_0 && (mcs[0]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[0] = idx;
		else if (pCurrTxRate->CurrMCS==MCS_1 && (mcs[1]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[1] = idx;
		else if (pCurrTxRate->CurrMCS==MCS_2 && (mcs[2]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[2] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_3)
			mcs[3] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_4)
			mcs[4] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_5)
			mcs[5] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_6)
			mcs[6] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_7) && (pCurrTxRate->ShortGI == GI_800))
			mcs[7] = idx;
#ifdef DOT11_N_SUPPORT
		else if (pCurrTxRate->CurrMCS == MCS_12)
			mcs[12] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_13)
			mcs[13] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_14)
			mcs[14] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_15) && (pCurrTxRate->ShortGI == GI_800))
		{
			mcs[15] = idx;
		}
#ifdef DOT11N_SS3_SUPPORT
		else if (pCurrTxRate->CurrMCS == MCS_20)
			mcs[20] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_21)
			mcs[21] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_22)
			mcs[22] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_23)
			mcs[23] = idx;
#endif /*  DOT11N_SS3_SUPPORT */
#endif /*  DOT11_N_SUPPORT */
	}

#ifdef DBG_CTRL_SUPPORT
	/*  Debug Option: Disable highest MCSs when picking initial MCS based on RSSI */
	if (pAd->CommonCfg.DebugFlags & DBF_INIT_MCS_DIS1)
		mcs[23] = mcs[15] = mcs[7] = mcs[22] = mcs[14] = mcs[6] = 0;
#endif /* DBG_CTRL_SUPPORT */
}


/*  MlmeClearTxQuality - Clear TxQuality history only for the active BF state */
VOID MlmeClearTxQuality(MAC_TABLE_ENTRY *pEntry)
{
		NdisZeroMemory(pEntry->TxQuality, sizeof(pEntry->TxQuality));

	NdisZeroMemory(pEntry->PER, sizeof(pEntry->PER));
}


/*  MlmeClearAllTxQuality - Clear both BF and non-BF TxQuality history */
VOID MlmeClearAllTxQuality(MAC_TABLE_ENTRY *pEntry)
{
	NdisZeroMemory(pEntry->TxQuality, sizeof(pEntry->TxQuality));

	NdisZeroMemory(pEntry->PER, sizeof(pEntry->PER));
}


/*  MlmeDecTxQuality - Decrement TxQuality of specified rate table entry */
VOID MlmeDecTxQuality(MAC_TABLE_ENTRY *pEntry, UCHAR rateIndex)
{
	if (pEntry->TxQuality[rateIndex])
		pEntry->TxQuality[rateIndex]--;
}


VOID MlmeSetTxQuality(MAC_TABLE_ENTRY *pEntry, UCHAR rate_idx, USHORT quality)
{
		pEntry->TxQuality[rate_idx] = quality;
}


USHORT MlmeGetTxQuality(MAC_TABLE_ENTRY *pEntry, UCHAR rateIndex)
{
	return pEntry->TxQuality[rateIndex];
}


#ifdef CONFIG_AP_SUPPORT
VOID APMlmeSetTxRate(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_LEGACY_TB *pTxRate)
{
	UCHAR tx_mode = pTxRate->Mode;
#ifdef DOT11_VHT_AC
	UCHAR tx_bw = pTxRate->BW;

	if ((pAd->chipCap.phy_caps & fPHY_CAP_VHT) && 
		((pEntry->pTable == RateTableVht2S) || (pEntry->pTable == RateTableVht1S) ||
		 (pEntry->pTable == RateTableVht1S_MCS9) ||
		 (pEntry->pTable == RateTableVht2S_BW20) ||
		 (pEntry->pTable == RateTableVht2S_BW40) ||
		 (pEntry->pTable == RateTableVht2S_MCS7)))
	{
		RTMP_RA_GRP_TB *pAdaptTbEntry = (RTMP_RA_GRP_TB *)pTxRate;
		UCHAR bw_cap = BW_20;
			
		if (pEntry->MaxHTPhyMode.field.BW != pAdaptTbEntry->BW)
		{
			switch (pEntry->MaxHTPhyMode.field.BW)
			{
				case BW_80:
					bw_cap = pAdaptTbEntry->BW;
					break;
				case BW_40:
					if (pAdaptTbEntry->BW == BW_80)
						bw_cap = BW_40;
					else
						bw_cap = pAdaptTbEntry->BW;
					break;
				//case BW_10:
					//bw_cap = BW_10;
					//break;
				case BW_20:
				default:
					if (pAdaptTbEntry->BW == BW_80 || pAdaptTbEntry->BW == BW_40)
						bw_cap = BW_20;
					else
						bw_cap = pAdaptTbEntry->BW;
					break;
			}
			tx_bw = bw_cap;
		}
		else
			tx_bw = pAdaptTbEntry->BW;

		if ((pEntry->force_op_mode == TRUE))
		{
			switch (pEntry->operating_mode.ch_width) {
				case 1:
					bw_cap = BW_40;
					break;
				case 2:
					bw_cap = BW_80;
					break;
				case 0:
				default:
					bw_cap = BW_20;
					break;
			}
			if ((tx_bw != BW_10) && (tx_bw >= bw_cap))
				tx_bw = bw_cap;
		}


#ifdef WFA_VHT_PF
		if (pAd->CommonCfg.vht_bw_signal && tx_bw == BW_40 &&
			pAdaptTbEntry->Mode == MODE_VHT &&
			(pAd->MacTab.fAnyStation20Only == FALSE))
		{
			// try to use BW_40 for VHT mode!
			tx_mode = pAdaptTbEntry->Mode;
		}
#endif /* WFA_VHT_PF */
DBGPRINT(RT_DEBUG_INFO, ("%s(): txbw=%d, txmode=%d\n", __FUNCTION__, tx_bw, tx_mode));
	}
#endif /* DOT11_VHT_AC */

#ifdef DOT11_N_SUPPORT
	if (tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD)
	{
		if ((pTxRate->STBC) && (pEntry->MaxHTPhyMode.field.STBC))
			pEntry->HTPhyMode.field.STBC = STBC_USE;
		else
			pEntry->HTPhyMode.field.STBC = STBC_NONE;

		if ((pTxRate->ShortGI || pAd->WIFItestbed.bShortGI) && (pEntry->MaxHTPhyMode.field.ShortGI))
			pEntry->HTPhyMode.field.ShortGI = GI_400;
		else
			pEntry->HTPhyMode.field.ShortGI = GI_800;
	}
		
	/* TODO: will check ldpc if related to rate table */
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE) ||
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE)) {
		pEntry->HTPhyMode.field.ldpc = TRUE;
	} else {
		pEntry->HTPhyMode.field.ldpc = FALSE;
	}

#ifdef DOT11_VHT_AC
	if (tx_mode == MODE_VHT)
	{
		if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI80_CAPABLE)) && 
			(pTxRate->ShortGI
#ifdef WFA_VHT_PF
			|| pAd->vht_force_sgi
#endif /* WFA_VHT_PF */
			)
		)
			pEntry->HTPhyMode.field.ShortGI = GI_400;
		else
			pEntry->HTPhyMode.field.ShortGI = GI_800;

		if (pTxRate->STBC && (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE)))
			pEntry->HTPhyMode.field.STBC = STBC_USE;
		else
			pEntry->HTPhyMode.field.STBC = STBC_NONE;
	}
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

	if (pTxRate->CurrMCS < MCS_AUTO)
		pEntry->HTPhyMode.field.MCS = pTxRate->CurrMCS;

	pEntry->HTPhyMode.field.MODE = tx_mode;

#ifdef DOT11_N_SUPPORT
	if ((pAd->WIFItestbed.bGreenField & pEntry->HTCapability.HtCapInfo.GF) && (pEntry->HTPhyMode.field.MODE == MODE_HTMIX))
	{
		/* force Tx GreenField */
		pEntry->HTPhyMode.field.MODE = MODE_HTGREENFIELD;
	}

	/* BW depends on BSSWidthTrigger and Negotiated BW */
	if (pAd->CommonCfg.bRcvBSSWidthTriggerEvents ||
		(pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ||
		(pEntry->MaxHTPhyMode.field.BW==BW_20) ||
		(pAd->CommonCfg.BBPCurrentBW==BW_20))
		pEntry->HTPhyMode.field.BW = BW_20;
	else
		pEntry->HTPhyMode.field.BW = BW_40;

#ifdef DOT11_VHT_AC
	if (pAd->CommonCfg.BBPCurrentBW==BW_80 &&
		pEntry->MaxHTPhyMode.field.BW == BW_80 &&
		pEntry->MaxHTPhyMode.field.MODE == MODE_VHT)
		pEntry->HTPhyMode.field.BW = BW_80;

#ifdef NEW_RATE_ADAPT_SUPPORT
	if ((pEntry->pTable == RateTableVht2S) ||
		(pEntry->pTable == RateTableVht2S_BW20) ||
		(pEntry->pTable == RateTableVht2S_BW40) ||
		(pEntry->pTable == RateTableVht1S) ||
		(pEntry->pTable == RateTableVht1S_MCS9) ||
		(pEntry->pTable == RateTableVht2S_MCS7))
	{
		RTMP_RA_GRP_TB *pAdaptTbEntry = (RTMP_RA_GRP_TB *)pTxRate;
		pEntry->HTPhyMode.field.MCS = pAdaptTbEntry->CurrMCS | ((pAdaptTbEntry->dataRate -1) <<4);		
		pEntry->HTPhyMode.field.BW = tx_bw;

#ifdef WFA_VHT_PF
		if ((pAd->vht_force_tx_stbc)
			&& (pEntry->HTPhyMode.field.MODE == MODE_VHT)
			&& (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE))
			&& (pEntry->HTPhyMode.field.STBC == STBC_NONE)
		)
		{
			pEntry->HTPhyMode.field.MCS = pAdaptTbEntry->CurrMCS;
			pEntry->HTPhyMode.field.STBC = STBC_USE;
		}
#endif /* WFA_VHT_PF */
	}
	else if (IS_VHT_STA(pEntry))
	{
		UCHAR bw_max = pEntry->MaxHTPhyMode.field.BW;
		if (pEntry->force_op_mode == TRUE)
		{
			switch (pEntry->operating_mode.ch_width) {
				case 1:
					bw_max = BW_40;
					break;
				case 2: /* not support for BW_80 for other rate table */
				case 0:
				default:
					bw_max = BW_20;
					break;
			}
		}

		if ( (bw_max != BW_10) &&
			(bw_max > pAd->CommonCfg.BBPCurrentBW))
		{
			bw_max = pAd->CommonCfg.BBPCurrentBW;
		}
		pEntry->HTPhyMode.field.BW = bw_max;		
	}
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
	if (pEntry->pTable == Ags2x2VhtRateTable)
	{
		RTMP_RA_AGS_TB *pAgsTbEntry = (RTMP_RA_AGS_TB *)pTxRate;
		pEntry->HTPhyMode.field.MCS = pAgsTbEntry->CurrMCS | (pAgsTbEntry->Nss <<4);
	}
#endif /* AGS_SUPPORT */
#endif /* DOT11_VHT_AC */

#ifdef RANGE_EXTEND
#ifdef NEW_RATE_ADAPT_SUPPORT
	/* 20 MHz Fallback */
	if ((tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD) &&
	    pEntry->HTPhyMode.field.BW == BW_40 &&
	    ADAPT_RATE_TABLE(pEntry->pTable))
	{
		if (pEntry->HTPhyMode.field.MCS == 32
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS0) == 0
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			/* Map HT Duplicate to 20MHz MCS0 */
			pEntry->HTPhyMode.field.BW = BW_20;
			pEntry->HTPhyMode.field.MCS = 0;
		}
		else if (pEntry->HTPhyMode.field.MCS == 0 &&
				(pAd->CommonCfg.DebugFlags & DBF_FORCE_20MHZ) == 0
#ifdef DBG_CTRL_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS1) == 0
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			/* Map 40MHz MCS0 to 20MHz MCS1 */
			pEntry->HTPhyMode.field.BW = BW_20;
			pEntry->HTPhyMode.field.MCS = 1;
		}
		else if (pEntry->HTPhyMode.field.MCS == 8
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_ENABLE_20MHZ_MCS8)
#endif /* DBG_CTRL_SUPPORT */
			)
		{
			/* Map 40MHz MCS8 to 20MHz MCS8 */
			pEntry->HTPhyMode.field.BW = BW_20;
		}
	}
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef DBG_CTRL_SUPPORT
	/* Debug Option: Force BW */
	if (pAd->CommonCfg.DebugFlags & DBF_FORCE_40MHZ)
	{
		pEntry->HTPhyMode.field.BW = BW_40;
	}
	else if (pAd->CommonCfg.DebugFlags & DBF_FORCE_20MHZ)
	{
		pEntry->HTPhyMode.field.BW = BW_20;
	}
#endif /* DBG_CTRL_SUPPORT */
#endif /* RANGE_EXTEND */

	/* Reexam each bandwidth's SGI support. */
	if ((pEntry->HTPhyMode.field.BW==BW_20 && !CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE)) ||
		(pEntry->HTPhyMode.field.BW==BW_40 && !CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE)) )
		pEntry->HTPhyMode.field.ShortGI = GI_800;

#ifdef DBG_CTRL_SUPPORT
	/* Debug option: Force Short GI */
	if (pAd->CommonCfg.DebugFlags & DBF_FORCE_SGI)
		pEntry->HTPhyMode.field.ShortGI = GI_400;
#endif /* DBG_CTRL_SUPPORT */
#endif /* DOT11_N_SUPPORT */

	pAd->LastTxRate = (USHORT)(pEntry->HTPhyMode.word);

#if defined(RTMP_MAC) || defined(RLT_MAC)
#ifdef FIFO_EXT_SUPPORT
	if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
		AsicFifoExtEntryClean(pAd, pEntry);
#endif /* FIFO_EXT_SUPPORT */
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MCS_LUT_SUPPORT
	asic_mcs_lut_update(pAd, pEntry);
	pEntry->LastTxRate = (USHORT) (pEntry->HTPhyMode.word);
#endif /* MCS_LUT_SUPPORT */


}
#endif /* CONFIG_AP_SUPPORT */

#ifdef MULTI_CLIENT_SUPPORT
extern INT MidRateRssi;
extern INT FarRateRssi;
#endif



VOID MlmeSelectTxRateTable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR **ppTable,
	IN UCHAR *pTableSize,
	IN UCHAR *pInitTxRateIdx)
{
#ifdef MULTI_CLIENT_SUPPORT
	INT Rssi = -127;
     
	Rssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);
#endif
     
	do
	{
#ifdef DOT11_VHT_AC
		INT mcs_idx, ss = 0, bw = pEntry->MaxHTPhyMode.field.BW;

		if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
			(pEntry->SupportRateMode & SUPPORT_VHT_MODE))
		{
			for (mcs_idx = 0; mcs_idx < MAX_LEN_OF_VHT_RATES; mcs_idx++)
			{
				if (pEntry->SupportVHTMCS[mcs_idx])
				{
					if (mcs_idx <= 9)
						ss =1;
					if (mcs_idx >= 10)
						ss = 2;
				}
			}

#ifdef WFA_VHT_PF
			if ((pAd->CommonCfg.vht_nss_cap > 0) &&
				(ss > pAd->CommonCfg.vht_nss_cap))
				ss = pAd->CommonCfg.vht_nss_cap;
#endif /* WFA_VHT_PF */

			if ((pEntry->force_op_mode == TRUE) && 
			    (pEntry->operating_mode.rx_nss_type == 0)) 
			{
				if (pEntry->operating_mode.rx_nss < pAd->CommonCfg.TxStream)
					ss = pEntry->operating_mode.rx_nss + 1;

				switch (pEntry->operating_mode.ch_width)
				{
					case 0:
						bw = BW_20;
						break;
					case 1:
						bw = BW_40;
						break;
					case 2:
					default:
						bw = BW_80;
						break;
				}
			}

#ifdef NEW_RATE_ADAPT_SUPPORT
			if (pAd->rateAlg == RATE_ALG_GRP) {
				if (ss == 2) {
					if (pEntry->MaxHTPhyMode.field.BW == BW_20) {
						*ppTable = RateTableVht2S_BW20;
					} else if (pEntry->MaxHTPhyMode.field.BW == BW_40) {
						*ppTable = RateTableVht2S_BW40;
					} else if ((pEntry->SupportVHTMCS[MCS_8] == 1) && 
						(pEntry->SupportVHTMCS[MCS_9] == 1)) {
						*ppTable = RateTableVht2S;
					} else {
						*ppTable = RateTableVht2S_MCS7;
					}
				} else if (ss == 1) {
					if ((pEntry->SupportVHTMCS[MCS_8] == 1) && 
						(pEntry->SupportVHTMCS[MCS_9] == 1))
						*ppTable = RateTableVht1S_MCS9;
					else
						*ppTable = RateTableVht1S;
					DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s(): Select RateTableVht%dS%s\n",
							__FUNCTION__, (ss == 2 ? 2 : 1),
							((*ppTable == RateTableVht1S_MCS9) ? "_MCS9" : "")));
				} else {
					DBGPRINT(RT_DEBUG_ERROR, ("%s:unknow ss(%d)\n", __FUNCTION__, ss));
				}
			}
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
			if (pAd->rateAlg == RATE_ALG_AGS)
			{
				if (ss == 2)
					*ppTable = Ags2x2VhtRateTable;
				else
					*ppTable = Ags1x1VhtRateTable;
				DBGPRINT(RT_DEBUG_TRACE, ("%s(): Select Ags%dx%dVhtRateTable\n",
							__FUNCTION__, (ss == 2 ? 2 : 1), (ss == 2 ? 2 : 1)));
			}
#endif /* AGS_SUPPORT */

			if ((IS_RT8592(pAd) && ( bw != BW_40)) ||
				(!IS_RT8592(pAd)))
				break;

#ifdef WFA_VHT_PF
			// TODO: shiang, add for Realtek behavior when run in BW signaling mode test and we are the testbed!
			// TODO: add at 11/15!
			if ((pAd->CommonCfg.vht_bw_signal == BW_SIGNALING_DYNAMIC) &&
				(bw == BW_40) &&
				(pAd->MacTab.fAnyStation20Only == FALSE))
				break;
#endif /* WFA_VHT_PF */
		}

		if (IS_RT8592(pAd) && 
			WMODE_CAP_AC(pAd->CommonCfg.PhyMode) && 
			(pEntry->SupportRateMode & SUPPORT_VHT_MODE) &&
			bw == BW_40 && (pAd->LatchRfRegs.Channel > 14)
		)
		{
			if (ss == 1) {
#ifdef NEW_RATE_ADAPT_SUPPORT
				if (pAd->rateAlg == RATE_ALG_GRP)
				{
#ifdef MULTI_CLIENT_SUPPORT
					if (is_multiclient_mode_on(pAd) && (Rssi > -65))
					{
						*ppTable = RateSwitchTableAdapt11N1SForMultiClients;
					}
					else
#endif
#ifdef INTERFERENCE_RA_SUPPORT
					if (is_interference_mode_on(pAd) && (Rssi > -65))
					{
						*ppTable = RateSwitchTableAdapt11N1SForInterference;
					}
					else
#endif
					{
						*ppTable = RateSwitchTableAdapt11N1S;
					}
				}
				else
#endif
					*ppTable = RateSwitchTable11N1SForABand;

			DBGPRINT(RT_DEBUG_TRACE, ("%s(): Select RateSwitchTable%s11N1S%s\n",
							__FUNCTION__, 
							((pAd->rateAlg == RATE_ALG_GRP) ? "Adapt" : ""),
							((pAd->rateAlg == RATE_ALG_GRP) ? "ForABand" : "")));
			}
			else if (ss == 2)
			{
#ifdef NEW_RATE_ADAPT_SUPPORT
				if (pAd->rateAlg == RATE_ALG_GRP) 
				{
#ifdef MULTI_CLIENT_SUPPORT
					if (is_multiclient_mode_on(pAd) && (Rssi > -65))
					{
						*ppTable = RateSwitchTableAdapt11N2SForMultiClients;
					}
					else
#endif
					{
						*ppTable = RateSwitchTableAdapt11N2S;
					}
				} else
#endif /* NEW_RATE_ADAPT_SUPPORT */
					*ppTable = RateSwitchTable11BGN2SForABand;
				DBGPRINT(RT_DEBUG_TRACE, ("%s(): Select RateSwitchTable%s11N2S%s\n",
							__FUNCTION__, 
							((pAd->rateAlg == RATE_ALG_GRP) ? "Adapt" : ""),
							((pAd->rateAlg == RATE_ALG_GRP) ? "ForABand" : "")));

			} else {
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): Invalid SS!\n", __FUNCTION__));
			}

			if (*ppTable)
				break;
		}
#endif /* DOT11_VHT_AC */


#ifdef DOT11_N_SUPPORT
		/*if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
		/*	((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
		if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
			(pEntry->HTCapability.MCSSet[0] != 0x00) &&
			((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)))
		{/* 11BGN 1S AP*/
#ifdef AGS_SUPPORT
			if (SUPPORT_AGS(pAd))
				*ppTable = AGS1x1HTRateTable;
			else
#endif /* AGS_SUPPORT */
#ifdef NEW_RATE_ADAPT_SUPPORT
			if (pAd->rateAlg == RATE_ALG_GRP)
			{
#ifdef MULTI_CLIENT_SUPPORT
				if (is_multiclient_mode_on(pAd) && (Rssi > -65))
				{
					*ppTable = RateSwitchTableAdapt11N1SForMultiClients;
 				}
				else
#endif
#ifdef INTERFERENCE_RA_SUPPORT
				if (is_interference_mode_on(pAd) && (Rssi > -65))
				{
					*ppTable = RateSwitchTableAdapt11N1SForInterference;
 				}
				else
#endif
				{
					*ppTable = RateSwitchTableAdapt11N1S;
				}
			}
			else
#endif
			if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
				*ppTable = RateSwitchTable11BGN1S;
			else
				*ppTable = RateSwitchTable11N1SForABand;

			break;
		}

#ifdef AGS_SUPPORT
		/* only for station */
		if (SUPPORT_AGS(pAd) && 
			(pEntry->HTCapability.MCSSet[0] != 0x00) && 
			(pEntry->HTCapability.MCSSet[1] != 0x00) && 
			(pEntry->HTCapability.MCSSet[2] != 0x00) && 
			(pAd->CommonCfg.TxStream == 3))
		{/* 11N 3S */
			*ppTable = AGS3x3HTRateTable;
			break;
		}
#endif /* AGS_SUPPORT */

		/*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
		/*	(pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
		if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) && 
			(pEntry->HTCapability.MCSSet[0] != 0x00) &&
			(pEntry->HTCapability.MCSSet[1] != 0x00) && 
#ifdef THERMAL_PROTECT_SUPPORT
			(pAd->force_one_tx_stream == FALSE) &&
#endif /* THERMAL_PROTECT_SUPPORT */
			(((pAd->Antenna.field.TxPath == 3) && (pEntry->HTCapability.MCSSet[2] == 0x00)) || (pAd->CommonCfg.TxStream == 2)))
		{/* 11BGN 2S AP*/
#ifdef AGS_SUPPORT
			if (SUPPORT_AGS(pAd))
			{
				*ppTable = AGS2x2HTRateTable;
			}
			else
#endif /* AGS_SUPPORT */
#ifdef NEW_RATE_ADAPT_SUPPORT
				if (pAd->rateAlg == RATE_ALG_GRP) {
#ifdef MULTI_CLIENT_SUPPORT
    				if (is_multiclient_mode_on(pAd) && (Rssi > -65))
    				{
#if defined(MT7603) || defined(MT7628)
					if (IS_MT7603(pAd) || (IS_MT7628(pAd))) {
						if ( pEntry->MmpsMode == MMPS_STATIC)
							*ppTable = RateSwitchTableAdapt11N1SForMultiClients;
						else
							*ppTable = RateSwitchTableAdapt11N2SForMultiClients;
					} else
#endif /* defined(MT7603) || defined(MT7628) */
    						*ppTable = RateSwitchTableAdapt11N2SForMultiClients;
    				}
    				else
#endif /* end MULTI_CLIENT_SUPPORT */
#ifdef INTERFERENCE_RA_SUPPORT
    				if (is_interference_mode_on(pAd) && (Rssi > -65))
    				{
#if defined(MT7603) || defined(MT7628)
					if (IS_MT7603(pAd) || (IS_MT7628(pAd))) {
						if ( pEntry->MmpsMode == MMPS_STATIC)
							*ppTable = RateSwitchTableAdapt11N1SForInterference;
						else
							*ppTable = RateSwitchTableAdapt11N2SForInterference;
					} else
#endif /* defined(MT7603) || defined(MT7628) */
    						*ppTable = RateSwitchTableAdapt11N2SForMultiClients;
    				}
    				else
#endif /* INTERFERENCE_RA_SUPPORT */
				{
#if defined(MT7603) || defined(MT7628)
					if (IS_MT7603(pAd) || (IS_MT7628(pAd))) {
						if ( pEntry->MmpsMode == MMPS_STATIC)
							*ppTable = RateSwitchTableAdapt11N1S;
						else
							*ppTable = RateSwitchTableAdapt11N2S;
					} else
#endif /* defined(MT7603) || defined(MT7628) */
						*ppTable = RateSwitchTableAdapt11N2S;
    					}
				} else
#endif /* NEW_RATE_ADAPT_SUPPORT */
			if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
					*ppTable = RateSwitchTable11BGN2S;
				else
					*ppTable = RateSwitchTable11BGN2SForABand;

			break;
		}

#ifdef DOT11N_SS3_SUPPORT
		if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
			(pEntry->HTCapability.MCSSet[0] != 0x00) &&
			(pEntry->HTCapability.MCSSet[1] != 0x00) &&
			(pEntry->HTCapability.MCSSet[2] != 0x00) &&
			(pAd->CommonCfg.TxStream == 3))
		{
#ifdef NEW_RATE_ADAPT_SUPPORT
			if (pAd->rateAlg == RATE_ALG_GRP)
			{
#if defined(MT7603) || defined(MT7628)
					if (IS_MT7603(pAd) || (IS_MT7628(pAd))) {
						if ( pEntry->MmpsMode == MMPS_STATIC )
						{
#ifdef MULTI_CLIENT_SUPPORT
							if (is_multiclient_mode_on(pAd) && (Rssi > -65))
					        	{
								*ppTable = RateSwitchTableAdapt11N1SForMultiClients;
							}
							else
#endif
#ifdef INTERFERENCE_RA_SUPPORT
							if (is_interfence_mode_on(pAd) && (Rssi > -65))
					        	{
								*ppTable = RateSwitchTableAdapt11N1SForInterference;
							}
							else
#endif
							{
								*ppTable = RateSwitchTableAdapt11N1S;
							}
						}
						else
						{
#ifdef MULTI_CLIENT_SUPPORT
							if (is_multiclient_mode_on(pAd) && (Rssi > -65))
					        	{
								*ppTable = RateSwitchTableAdapt11N2SForMultiClients;
							}
						else
#endif
#ifdef INTERFERENCE_RA_SUPPORT
							if (is_multiclient_mode_on(pAd) && (Rssi > -65))
					        	{
								*ppTable = RateSwitchTableAdapt11N2SForInterference;
							}
							else
#endif
							{
								*ppTable = RateSwitchTableAdapt11N2S;
							}
						}
					} else
#endif /* defined(MT7603) || defined(MT7628) */
						*ppTable = RateSwitchTableAdapt11N3S;
			}
			else
#endif /* NEW_RATE_ADAPT_SUPPORT */
			if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
				*ppTable = RateSwitchTable11N3S;
			else
				*ppTable = RateSwitchTable11N3SForABand;

			break;
		}
#endif /* DOT11N_SS3_SUPPORT */

		/*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && ((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
		if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
			((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)
#ifdef THERMAL_PROTECT_SUPPORT
			|| (pAd->force_one_tx_stream == TRUE)
#endif /* THERMAL_PROTECT_SUPPORT */
			))
		{/* 11N 1S AP*/
#ifdef AGS_SUPPORT
			if (SUPPORT_AGS(pAd))
				*ppTable = AGS1x1HTRateTable;
			else
#endif /* AGS_SUPPORT */
			{
#ifdef NEW_RATE_ADAPT_SUPPORT
				if (pAd->rateAlg == RATE_ALG_GRP)
				{
#ifdef MULTI_CLIENT_SUPPORT
					if (is_multiclient_mode_on(pAd) && (Rssi > -65))
				        {
						*ppTable = RateSwitchTableAdapt11N1SForMultiClients;
					}
					else
#endif
#ifdef INTERFERENCE_RA_SUPPORT
					if (is_interference_mode_on(pAd) && (Rssi > -65))
				        {
						*ppTable = RateSwitchTableAdapt11N1SForInterference;
					}
					else
#endif
					{
						*ppTable = RateSwitchTableAdapt11N1S;
					}
				}
				else
#endif /* NEW_RATE_ADAPT_SUPPORT */
				if (pAd->LatchRfRegs.Channel <= 14)
				{
#ifdef MULTI_CLIENT_SUPPORT
					if (is_multiclient_mode_on(pAd) && (Rssi > -65))
				        {
						*ppTable = RateSwitchTableAdapt11N1SForMultiClients;
					}
				else
#endif
#ifdef INTERFERENCE_RA_SUPPORT
					if (is_interference_mode_on(pAd) && (Rssi > -65))
					{
						*ppTable = RateSwitchTableAdapt11N1SForInterference;
 					}
					else
#endif
					{
						*ppTable = RateSwitchTableAdapt11N1S;
					}
				}
				else
					{
						//printk("--->%s,%d\n", __func__, __LINE__);
						*ppTable = RateSwitchTable11N1SForABand;
				}
			}
			break;
		}

		/*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
		if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
			(pEntry->HTCapability.MCSSet[1] != 0x00) && 
			(pAd->CommonCfg.TxStream == 2))
		{/* 11N 2S AP*/
#ifdef AGS_SUPPORT
			if (SUPPORT_AGS(pAd))
				*ppTable = AGS2x2HTRateTable;
			else
#endif /* AGS_SUPPORT */
			{
#ifdef NEW_RATE_ADAPT_SUPPORT
				if (pAd->rateAlg == RATE_ALG_GRP) {
#ifdef MULTI_CLIENT_SUPPORT
					if (is_multiclient_mode_on(pAd) && (Rssi > -65))
					{
						*ppTable = RateSwitchTableAdapt11N2SForMultiClients;
 					}
					else
#endif
#ifdef INTERFERENCE_RA_SUPPORT
					if (is_interference_mode_on(pAd) && (Rssi > -65))
					{
						*ppTable = RateSwitchTableAdapt11N2SForInterference;
 					}
					else
#endif
					{
						*ppTable = RateSwitchTableAdapt11N2S;
					}
				} else
#endif /* NEW_RATE_ADAPT_SUPPORT */
				if (pAd->LatchRfRegs.Channel <= 14)
				{
#ifdef MULTI_CLIENT_SUPPORT
					if (is_multiclient_mode_on(pAd) && (Rssi > -65))
				        {
						*ppTable = RateSwitchTableAdapt11N2SForMultiClients;
					}
					else
#endif
#ifdef INTERFERENCE_RA_SUPPORT
					if (is_interference_mode_on(pAd) && (Rssi > -65))
				        {
						*ppTable = RateSwitchTableAdapt11N2SForInterference;
					}
					else
#endif
					{
						*ppTable = RateSwitchTableAdapt11N2S;
					}
				}
				else
					*ppTable = RateSwitchTable11N2SForABand;			
			}
			break;
		}

#ifdef DOT11N_SS3_SUPPORT
		if ((pEntry->HTCapability.MCSSet[0] != 0x00) &&
			(pEntry->HTCapability.MCSSet[1] != 0x00) &&
			(pEntry->HTCapability.MCSSet[2] != 0x00) &&
			(pAd->CommonCfg.TxStream == 3))
		{
#ifdef NEW_RATE_ADAPT_SUPPORT
			if (pAd->rateAlg == RATE_ALG_GRP)
				*ppTable = RateSwitchTableAdapt11N3S;
			else
#endif /* NEW_RATE_ADAPT_SUPPORT */
			{
				if (pAd->LatchRfRegs.Channel <= 14)
				*ppTable = RateSwitchTable11N3S;
				else
					*ppTable = RateSwitchTable11N3SForABand;
			}
			break;
		}
#endif /* DOT11N_SS3_SUPPORT */

#ifdef DOT11N_SS3_SUPPORT
		if (pAd->CommonCfg.TxStream == 3)
		{
			if  (pEntry->HTCapability.MCSSet[0] != 0x00)
			{
				if (pEntry->HTCapability.MCSSet[1] == 0x00)
				{	/* Only support 1SS */
					if (pEntry->RateLen > 0)
					{
#ifdef NEW_RATE_ADAPT_SUPPORT
					if (pAd->rateAlg == RATE_ALG_GRP)
					{
#ifdef MULTI_CLIENT_SUPPORT
						if (is_multiclient_mode_on(pAd) && (Rssi > -65))
					        {
							*ppTable = RateSwitchTableAdapt11N1SForMultiClients;
						}
						else
#endif
						{
						*ppTable = RateSwitchTableAdapt11N1S;
						}
					}
					else
#endif /* NEW_RATE_ADAPT_SUPPORT */
						if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
							*ppTable = RateSwitchTable11N1S;
						else
							*ppTable = RateSwitchTable11N1SForABand;	
					}
					else
					{
#ifdef NEW_RATE_ADAPT_SUPPORT
						if (pAd->rateAlg == RATE_ALG_GRP)
						{
#ifdef MULTI_CLIENT_SUPPORT
							if (is_multiclient_mode_on(pAd) && (Rssi > -65))
							{
								*ppTable = RateSwitchTableAdapt11N1SForMultiClients;
							}
							else
#endif
							{
								*ppTable = RateSwitchTableAdapt11N1S;
							}
					}
						else
#endif /* NEW_RATE_ADAPT_SUPPORT */
						if (pAd->LatchRfRegs.Channel <= 14)
							*ppTable = RateSwitchTable11N1S;
						else
							*ppTable = RateSwitchTable11N1SForABand;
					}
					break;
				}
				else if (pEntry->HTCapability.MCSSet[2] == 0x00)
				{	/* Only support 2SS */
					if (pEntry->RateLen > 0)
					{
#ifdef NEW_RATE_ADAPT_SUPPORT
						if (pAd->rateAlg == RATE_ALG_GRP)
						{
#ifdef MULTI_CLIENT_SUPPORT
							if (is_multiclient_mode_on(pAd) && (Rssi > -65))
							{
								*ppTable = RateSwitchTableAdapt11N2SForMultiClients;
							}
							else
#endif
							{
								*ppTable = RateSwitchTableAdapt11N2S;
							}
						}
						else
#endif /* NEW_RATE_ADAPT_SUPPORT */
						if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
							*ppTable = RateSwitchTable11BGN2S;
						else
							*ppTable = RateSwitchTable11BGN2SForABand;
					}
					else
					{
#ifdef NEW_RATE_ADAPT_SUPPORT
						if (pAd->rateAlg == RATE_ALG_GRP)
						{
#ifdef MULTI_CLIENT_SUPPORT
							if (is_multiclient_mode_on(pAd) && (Rssi > -65))
							{
								*ppTable = RateSwitchTableAdapt11N2SForMultiClients;
							}
							else
#endif
							{
								*ppTable = RateSwitchTableAdapt11N2S;
							}
						}
						else
#endif /* NEW_RATE_ADAPT_SUPPORT */
						if (pAd->LatchRfRegs.Channel <= 14)
							*ppTable = RateSwitchTable11N2S;
						else
							*ppTable = RateSwitchTable11N2SForABand;
					}
					break;
				}
				/* For 3SS case, we use the new rate table, so don't care it here */
			}
		}
#endif /* DOT11N_SS3_SUPPORT */
#endif /* DOT11_N_SUPPORT */

		if (((pEntry->SupportRateMode == SUPPORT_CCK_MODE) || 
			WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
#ifdef DOT11_N_SUPPORT
		/*Iverson mark for Adhoc b mode,sta will use rate 54  Mbps when connect with sta b/g/n mode */
		/* && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)*/
#endif /* DOT11_N_SUPPORT */
			)
		{/* B only AP*/
#ifdef NEW_RATE_ADAPT_SUPPORT
			if (pAd->rateAlg == RATE_ALG_GRP)
			{
				*ppTable = RateSwitchTableAdapt11B;
			}
			else
#endif /* NEW_RATE_ADAPT_SUPPORT */
			*ppTable = RateSwitchTable11B;			
			break;
		}

		/*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen > 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
		if ((pEntry->SupportRateMode & (SUPPORT_CCK_MODE)) &&
			(pEntry->SupportRateMode & (SUPPORT_OFDM_MODE))
#ifdef DOT11_N_SUPPORT
			&& (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
			)
		{/* B/G  mixed AP*/
#ifdef NEW_RATE_ADAPT_SUPPORT
			if (pAd->rateAlg == RATE_ALG_GRP)
			{
				*ppTable = RateSwitchTableAdapt11BG;
			}
			else
#endif /* NEW_RATE_ADAPT_SUPPORT */
			*ppTable = RateSwitchTable11BG;			
			break;
		}

		/*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
		if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) 
#ifdef DOT11_N_SUPPORT
			&& (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
			)
		{/* G only AP*/
#ifdef NEW_RATE_ADAPT_SUPPORT
			if (pAd->rateAlg == RATE_ALG_GRP)
			{
				*ppTable = RateSwitchTableAdapt11G;
			}
			else
#endif /* NEW_RATE_ADAPT_SUPPORT */
			*ppTable = RateSwitchTable11G;			
			break;
		}
#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef DOT11N_SS3_SUPPORT
			if (pAd->CommonCfg.TxStream >= 3)
			{
#ifdef NEW_RATE_ADAPT_SUPPORT
				if (pAd->rateAlg == RATE_ALG_GRP)
				{
					if (pEntry->HTCapability.MCSSet[2] == 0) {
#ifdef MULTI_CLIENT_SUPPORT
						if (is_multiclient_mode_on(pAd) && (Rssi > -65))
						{
							*ppTable = RateSwitchTableAdapt11N2SForMultiClients;
						}
						else
#endif
						{
							*ppTable = RateSwitchTableAdapt11N2S;
						}
					} else
					{
						*ppTable = RateSwitchTableAdapt11N3S;
					}
				}
				else
#endif /* NEW_RATE_ADAPT_SUPPORT */
				{
					if (pEntry->HTCapability.MCSSet[2] == 0)
						*ppTable = RateSwitchTable11N2S;
					else
						*ppTable = RateSwitchTable11N3S;
				}
			}
			else
#endif /* DOT11N_SS3_SUPPORT */
			{
				/*
					Temp solution for:
					EX: when the extend rate only supports 6, 12, 24 in
					the association req frame. So the pEntry->RateLen is 7.
				*/
				if (pAd->LatchRfRegs.Channel <= 14)
					*ppTable = RateSwitchTable11BG;
				else
					*ppTable = RateSwitchTable11G;
			}
			break;
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

	} while(FALSE);

#ifdef MULTI_CLIENT_SUPPORT
		Rssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);
 		if (is_multiclient_mode_on(pAd) > 0)
		{
			if (Rssi < MidRateRssi && Rssi >= FarRateRssi)
			{
				*ppTable = RateSwitchTableAdapt11N1SForMultiClients;		
			}
			else if(Rssi < FarRateRssi)
			{
				*ppTable = RateSwitchTableAdapt11BG;
			}
		}
#endif


	*pTableSize = RATE_TABLE_SIZE(*ppTable);
	*pInitTxRateIdx = RATE_TABLE_INIT_INDEX(*ppTable);


	/* Avoid rate index over range */
	if (pEntry->CurrTxRateIndex >= RATE_TABLE_SIZE(*ppTable))
		pEntry->CurrTxRateIndex = RATE_TABLE_SIZE(*ppTable) - 1;
}



/*
	MlmeSelectTxRate - select the MCS based on the RSSI and the available MCSs
		pAd - pointer to adapter
		pEntry - pointer to MAC table entry
		mcs - table of MCS index into the Rate Table. -1 => not supported
		Rssi - the Rssi value
		RssiOffset - offset to apply to the Rssi
*/
UCHAR MlmeSelectTxRate(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN CHAR		mcs[],
	IN CHAR		Rssi,
	IN CHAR		RssiOffset)
{
	UCHAR TxRateIdx = 0;
	UCHAR *pTable = pEntry->pTable;

#ifdef DOT11_N_SUPPORT
#ifdef NEW_RATE_ADAPT_SUPPORT
#ifdef DOT11_VHT_AC
	if (pTable == RateTableVht2S || pTable == RateTableVht2S_BW20 || pTable == RateTableVht2S_BW40
		|| (pTable == RateTableVht2S_MCS7))
	{
		/*  VHT mode with 2SS */
		if (mcs[15]>=0 && (Rssi >= (-70+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_15]))
			TxRateIdx = mcs[15];
		else if (mcs[14]>=0 && (Rssi >= (-72+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_14]))
			TxRateIdx = mcs[14];
		else if (mcs[13]>=0 && (Rssi >= (-76+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_13]))
			TxRateIdx = mcs[13];
		else if (mcs[12]>=0 && (Rssi >= (-78+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_12]))
			TxRateIdx = mcs[12];
		else if (mcs[4]>=0 && (Rssi >= (-82+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_4]))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi >= (-84+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_3]))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi >= (-86+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_2]))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi >= (-88+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_1]))
			TxRateIdx = mcs[1];
		else if (mcs[0]>=0 )
			TxRateIdx = mcs[0];
	}
	else if (pTable == RateTableVht1S_MCS9)
	{	/*  VHT mode with 1SS */
		if (mcs[9]>=0 && (Rssi > (-72+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_9]))
			TxRateIdx = mcs[8];
		else if (mcs[8]>=0 && (Rssi > (-72+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_8]))
			TxRateIdx = mcs[8];
		else if (mcs[7]>=0 && (Rssi > (-72+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_7]))
			TxRateIdx = mcs[7];
		else if (mcs[6]>=0 && (Rssi > (-74+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_6]))
			TxRateIdx = mcs[6];
		else if (mcs[5]>=0 && (Rssi > (-77+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_5]))
			TxRateIdx = mcs[5];
		else if (mcs[4]>=0 && (Rssi > (-79+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_4]))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi > (-81+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_3]))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi > (-83+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_2]))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi > (-86+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_1]))
			TxRateIdx = mcs[1];
		else if (mcs[0]>=0 )
			TxRateIdx = mcs[0];
	}
	else if (pTable == RateTableVht1S)
	{	/*  VHT mode with 1SS */
		if (mcs[7]>=0 && (Rssi > (-72+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_7]))
			TxRateIdx = mcs[7];
		else if (mcs[6]>=0 && (Rssi > (-74+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_6]))
			TxRateIdx = mcs[6];
		else if (mcs[5]>=0 && (Rssi > (-77+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_5]))
			TxRateIdx = mcs[5];
		else if (mcs[4]>=0 && (Rssi > (-79+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_4]))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi > (-81+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_3]))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi > (-83+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_2]))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi > (-86+RssiOffset)) && (pEntry->SupportVHTMCS[MCS_1]))
			TxRateIdx = mcs[1];
		else if (mcs[0]>=0 )
			TxRateIdx = mcs[0];
	}
	else
#endif /* DOT11_VHT_AC */
#endif /* NEW_RATE_ADAPT_SUPPORT */
#ifdef DOT11N_SS3_SUPPORT
	if ((pTable == RateSwitchTable11BGN3S) || (pTable == RateSwitchTable11N3S) || (pTable == RateSwitchTable11BGN3SForABand)
#ifdef NEW_RATE_ADAPT_SUPPORT
		|| (pTable == RateSwitchTableAdapt11N3S)
#endif /* NEW_RATE_ADAPT_SUPPORT */
	)
	{/*  N mode with 3 stream */
		if (mcs[23]>=0 && (Rssi >= (-66+RssiOffset)) && (pEntry->SupportHTMCS[MCS_23]))
			TxRateIdx = mcs[23];
		else if (mcs[22]>=0 && (Rssi >= (-70+RssiOffset)) && (pEntry->SupportHTMCS[MCS_22]))
			TxRateIdx = mcs[22];
		else if (mcs[21]>=0 && (Rssi >= (-72+RssiOffset)) && (pEntry->SupportHTMCS[MCS_21]))
			TxRateIdx = mcs[21];
		else if (mcs[20]>=0 && (Rssi >= (-74+RssiOffset)) && (pEntry->SupportHTMCS[MCS_20]))
			TxRateIdx = mcs[20];
		else if (mcs[13]>=0 && (Rssi >= (-76+RssiOffset)) && (pEntry->SupportHTMCS[MCS_13]))
			TxRateIdx = mcs[13];
		else if (mcs[12]>=0 && (Rssi >= (-78+RssiOffset)) && (pEntry->SupportHTMCS[MCS_12]))
			TxRateIdx = mcs[12];
		else if (mcs[4]>=0 && (Rssi >= (-82+RssiOffset)) && (pEntry->SupportHTMCS[MCS_4]))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi >= (-84+RssiOffset)) && (pEntry->SupportHTMCS[MCS_3]))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi >= (-86+RssiOffset)) && (pEntry->SupportHTMCS[MCS_2]))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi >= (-88+RssiOffset)) && (pEntry->SupportHTMCS[MCS_1]))
			TxRateIdx = mcs[1];
		else if (mcs[0]>=0 )
			TxRateIdx = mcs[0];
	}
	else
#endif /*  DOT11N_SS3_SUPPORT */
	if ((pTable == RateSwitchTable11BGN2S) || (pTable == RateSwitchTable11BGN2SForABand) ||
		(pTable == RateSwitchTable11N2S) || (pTable == RateSwitchTable11N2SForABand)
#ifdef NEW_RATE_ADAPT_SUPPORT
		|| (pTable == RateSwitchTableAdapt11N2S)
#endif /* NEW_RATE_ADAPT_SUPPORT */
	)
	{/*  N mode with 2 stream */
		if (mcs[15]>=0 && (Rssi >= (-70+RssiOffset)) && (pEntry->SupportHTMCS[MCS_15]))
			TxRateIdx = mcs[15];
		else if (mcs[14]>=0 && (Rssi >= (-72+RssiOffset)) && (pEntry->SupportHTMCS[MCS_14]))
			TxRateIdx = mcs[14];
		else if (mcs[13]>=0 && (Rssi >= (-76+RssiOffset)) && (pEntry->SupportHTMCS[MCS_13]))
			TxRateIdx = mcs[13];
		else if (mcs[12]>=0 && (Rssi >= (-78+RssiOffset)) && (pEntry->SupportHTMCS[MCS_12]))
			TxRateIdx = mcs[12];
		else if (mcs[4]>=0 && (Rssi >= (-82+RssiOffset)) && (pEntry->SupportHTMCS[MCS_4]))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi >= (-84+RssiOffset)) && (pEntry->SupportHTMCS[MCS_3]))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi >= (-86+RssiOffset)) && (pEntry->SupportHTMCS[MCS_2]))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi >= (-88+RssiOffset)) && (pEntry->SupportHTMCS[MCS_1]))
			TxRateIdx = mcs[1];
		else if (mcs[0]>=0 )
			TxRateIdx = mcs[0];
	}
	else if ((pTable == RateSwitchTable11BGN1S) ||
			 (pTable == RateSwitchTable11N1S) ||
			 (pTable == RateSwitchTable11N1SForABand)
#ifdef NEW_RATE_ADAPT_SUPPORT
			|| (pTable == RateSwitchTableAdapt11N1S)
#endif /* NEW_RATE_ADAPT_SUPPORT */
	)
	{/*  N mode with 1 stream */
		{
			if (mcs[7]>=0 && (Rssi > (-72+RssiOffset)) && (pEntry->SupportHTMCS[MCS_7]))
				TxRateIdx = mcs[7];
			else if (mcs[6]>=0 && (Rssi > (-74+RssiOffset)) && (pEntry->SupportHTMCS[MCS_6]))
				TxRateIdx = mcs[6];
			else if (mcs[5]>=0 && (Rssi > (-77+RssiOffset)) && (pEntry->SupportHTMCS[MCS_5]))
				TxRateIdx = mcs[5];
			else if (mcs[4]>=0 && (Rssi > (-79+RssiOffset)) && (pEntry->SupportHTMCS[MCS_4]))
				TxRateIdx = mcs[4];
			else if (mcs[3]>=0 && (Rssi > (-81+RssiOffset)) && (pEntry->SupportHTMCS[MCS_3]))
				TxRateIdx = mcs[3];
			else if (mcs[2]>=0 && (Rssi > (-83+RssiOffset)) && (pEntry->SupportHTMCS[MCS_2]))
				TxRateIdx = mcs[2];
			else if (mcs[1]>=0 && (Rssi > (-86+RssiOffset)) && (pEntry->SupportHTMCS[MCS_1]))
				TxRateIdx = mcs[1];
			else if (mcs[0]>=0 )
				TxRateIdx = mcs[0];
		}
	}
	else
#endif /*  DOT11_N_SUPPORT */
	{/*  Legacy mode */
		if (mcs[7]>=0 && (Rssi > -70) && (pEntry->SupportOFDMMCS[MCS_7]))
		TxRateIdx = mcs[7];
		else if (mcs[6]>=0 && (Rssi > -74) && (pEntry->SupportOFDMMCS[MCS_7]))
			TxRateIdx = mcs[6];
		else if (mcs[5]>=0 && (Rssi > -78) && (pEntry->SupportOFDMMCS[MCS_7]))
			TxRateIdx = mcs[5];
		else if (mcs[4]>=0 && (Rssi > -82) && (pEntry->SupportOFDMMCS[MCS_7]))
			TxRateIdx = mcs[4];
		else if (mcs[4] == -1)							/*  for B-only mode */
		{
			if (mcs[3]>=0 && (Rssi > -85) && (pEntry->SupportCCKMCS[MCS_3]))
				TxRateIdx = mcs[3];
			else if (mcs[2]>=0 && (Rssi > -87) && (pEntry->SupportCCKMCS[MCS_2]))
				TxRateIdx = mcs[2];
			else if (mcs[1]>=0 && (Rssi > -90) && (pEntry->SupportCCKMCS[MCS_1]))
				TxRateIdx = mcs[1];
			else if (pEntry->SupportCCKMCS[MCS_0])
				TxRateIdx = mcs[0];
			else if (mcs[3]>=0 )
			TxRateIdx = mcs[3];
		}
		else if (mcs[3]>=0 && (Rssi > -85) && (pEntry->SupportOFDMMCS[MCS_3]))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi > -87) && (pEntry->SupportOFDMMCS[MCS_2]))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi > -90) && (pEntry->SupportOFDMMCS[MCS_1]))
			TxRateIdx = mcs[1];
		else if (mcs[0]>=0 )
			TxRateIdx = mcs[0];
	}


	return TxRateIdx;
}


/*  MlmeRAInit - Initialize Rate Adaptation for this entry */
VOID MlmeRAInit(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
#ifdef NEW_RATE_ADAPT_SUPPORT
	MlmeSetMcsGroup(pAd, pEntry);

	//pEntry->lastRateIdx = 1;
	pEntry->lastRateIdx = 0xFF;
	pEntry->lowTrafficCount = 0;
	pEntry->perThrdAdj = PER_THRD_ADJ;
	pEntry->TrafficLoading = RA_INIT_STATE;
	pEntry->RaHoldTime = 0;
#endif /* NEW_RATE_ADAPT_SUPPORT */


	pEntry->fLastSecAccordingRSSI = FALSE;
	pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
	pEntry->CurrTxRateIndex = 0;
	pEntry->TxRateUpPenalty = 0;

	MlmeClearAllTxQuality(pEntry);
}


/* #define TIMESTAMP_RA_LOG	*/ /* Include timestamp in RA Log */

/*
	MlmeRALog - Prints concise Rate Adaptation log entry
		The BF percentage counters are also updated
*/
VOID MlmeRALog(
	IN PRTMP_ADAPTER	pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN RA_LOG_TYPE		raLogType,
	IN ULONG			TxErrorRatio,
	IN ULONG			TxTotalCnt)
{
#ifdef TIMESTAMP_RA_LOG
	ULONG newTime;
	static ULONG saveRATime;
	struct timeval tval;

	do_gettimeofday(&tval);
	newTime = (tval.tv_sec*1000000L + tval.tv_usec);
#endif

	if (TxTotalCnt !=0 || raLogType==RAL_QUICK_DRS
#ifdef DBG_CTRL_SUPPORT
		|| (pAd->CommonCfg.DebugFlags & DBF_SHOW_ZERO_RA_LOG)
#endif /* DBG_CTRL_SUPPORT */
	)
	{
		BOOLEAN stbc, csd=FALSE;
		ULONG tp;

		/*  Get STBC and StreamMode state */
		stbc = (pEntry->HTPhyMode.field.STBC && pEntry->HTPhyMode.field.MCS<8);

#ifdef STREAM_MODE_SUPPORT
		if (pEntry->StreamModeMACReg != 0)
		{
			ULONG streamWord;

			RTMP_IO_READ32(pAd, pEntry->StreamModeMACReg+4, &streamWord);
			if (pEntry->HTPhyMode.field.MCS < 8)
				csd = (streamWord & 0x30000)==0x30000;
			else if (pEntry->HTPhyMode.field.MCS < 16)
				csd = (streamWord & 0xC0000)==0xC0000;
		}
#endif /* STREAM_MODE_SUPPORT */

		/*  Normalized throughput - packets per RA Interval */
		if (raLogType==RAL_QUICK_DRS)
			tp = (100-TxErrorRatio)*TxTotalCnt*RA_INTERVAL/(100*pAd->ra_fast_interval);
		else if (pEntry->LastSecTxRateChangeAction==RATE_NO_CHANGE
#ifdef DBG_CTRL_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_FORCE_QUICK_DRS)==0
#endif /* DBG_CTRL_SUPPORT */
		)
			tp = (100-TxErrorRatio)*TxTotalCnt/100;
		else
			tp = (100-TxErrorRatio)*TxTotalCnt*RA_INTERVAL/(100*(RA_INTERVAL-pAd->ra_fast_interval));

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_RA_LOG)
		{
			struct {
				USHORT phyMode;
				USHORT per;
				USHORT tp;
				USHORT bfRatio;
			} raLogInfo;

			raLogInfo.phyMode = pEntry->HTPhyMode.word;
			raLogInfo.per = TxErrorRatio;
			raLogInfo.tp = tp;
			dbQueueEnqueue(0x7e, (UCHAR *)&raLogInfo);
		}
		else
#endif /*  INCLUDE_DEBUG_QUEUE */
#endif /*  DBG_CTRL_SUPPORT */
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR,("%s[%d]: M=%d %c%c%c%c- PER=%ld%% TP=%ld ",
				raLogType==RAL_QUICK_DRS? " Q": (raLogType==RAL_NEW_DRS? "\nRA": "\nra"),
				pEntry->wcid, pEntry->HTPhyMode.field.MCS,
				pEntry->HTPhyMode.field.MODE==MODE_CCK? 'C': (pEntry->HTPhyMode.field.ShortGI? 'S': 'L'),
				pEntry->HTPhyMode.field.BW? '4': '2',
				stbc? 'S': 's',
				csd? 'C': 'c',
				TxErrorRatio, tp) );
		}
	}

#ifdef TIMESTAMP_RA_LOG
	saveRATime = newTime;
#endif
}


/*  MlmeRestoreLastRate - restore last saved rate */
VOID MlmeRestoreLastRate(
	IN PMAC_TABLE_ENTRY	pEntry)
{
	pEntry->CurrTxRateIndex = pEntry->lastRateIdx;
}


#ifdef DOT11N_SS3_SUPPORT
/*  MlmeCheckRDG - check if RDG should be enabled or disabled */
VOID MlmeCheckRDG(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY	pEntry)
{
	PUCHAR pTable = pEntry->pTable;

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
		return;
	}

	/*  Turn off RDG when 3s and rx count > tx count*5 */
	if (((pTable == RateSwitchTable11BGN3S) || 
		(pTable == RateSwitchTable11BGN3SForABand) || 
		(pTable == RateSwitchTable11N3S)
#ifdef NEW_RATE_ADAPT_SUPPORT
		|| (pTable == RateSwitchTableAdapt11N3S)
#endif /* NEW_RATE_ADAPT_SUPPORT */
		) && pAd->RalinkCounters.OneSecReceivedByteCount > 50000 &&
		pAd->RalinkCounters.OneSecTransmittedByteCount > 50000 &&
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE))
	{
		TX_LINK_CFG_STRUC TxLinkCfg;
		UINT32 TxOpThres;
		UCHAR				TableStep;
		RTMP_RA_LEGACY_TB *pTempTxRate;

#ifdef NEW_RATE_ADAPT_SUPPORT
		TableStep = ADAPT_RATE_TABLE(pTable)? 10: 5;
#else
		TableStep = 5;
#endif

		pTempTxRate = (RTMP_RA_LEGACY_TB *)(&pTable[(pEntry->CurrTxRateIndex + 1)*TableStep]);
		RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
		if (pAd->RalinkCounters.OneSecReceivedByteCount > (pAd->RalinkCounters.OneSecTransmittedByteCount * 5) &&
				pTempTxRate->CurrMCS != 23 && pTempTxRate->ShortGI != 1)
		{
			if (TxLinkCfg.field.TxRDGEn == 1)
			{
				TxLinkCfg.field.TxRDGEn = 0;
				RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);
				RTMP_IO_READ32(pAd, TXOP_THRES_CFG, &TxOpThres);
				TxOpThres |= 0xff00;
				RTMP_IO_WRITE32(pAd, TXOP_THRES_CFG, TxOpThres);
				DBGPRINT_RAW(RT_DEBUG_WARN,("DRS: RDG off!\n"));
			}
		}
		else
		{
			if (TxLinkCfg.field.TxRDGEn == 0)
			{
				TxLinkCfg.field.TxRDGEn = 1;
				RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);
				RTMP_IO_READ32(pAd, TXOP_THRES_CFG, &TxOpThres);
				TxOpThres &= 0xffff00ff;
				RTMP_IO_WRITE32(pAd, TXOP_THRES_CFG, TxOpThres);
				DBGPRINT_RAW(RT_DEBUG_WARN,("DRS: RDG on!\n"));
			}
		}
	}
}
#endif /*  DOT11N_SS3_SUPPORT */




#ifdef NEW_RATE_ADAPT_SUPPORT
UCHAR ra_get_lowest_rate(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	UCHAR *pTable = pEntry->pTable;
	RTMP_RA_GRP_TB *pNextTxRate;
	UCHAR TxRateIndex, NextTxRateIndex;

	if (ADAPT_RATE_TABLE(pTable))
	{
		NextTxRateIndex = RATE_TABLE_INIT_INDEX(pTable);
	
		do {
			TxRateIndex = NextTxRateIndex;
			/*  Get pointer to CurrTxRate entry */
			pNextTxRate = PTX_RA_GRP_ENTRY(pTable, TxRateIndex);
#ifdef GN_ONLY_AP_SUPPORT
            if((pAd->OpMode == OPMODE_STA) || (pAd->OpMode == OPMODE_AP)) {
			    CHAR Rssi;
			    Rssi = RTMPMaxRssi(pAd, pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1], pEntry->RssiSample.AvgRssi[2]);

			    if(((pAd->CommonCfg.bExcludeBRate == TRUE) && (PTX_RA_GRP_ENTRY(pTable, pNextTxRate->downMcs)->Mode == MODE_CCK)) ||
				   ((Rssi > -65) && pAd->LowerMcsValue))
				    break;
			}
#endif			
			NextTxRateIndex = pNextTxRate->downMcs;
		} while ( TxRateIndex != NextTxRateIndex );
	} else {
		TxRateIndex = 0;
	}

	return TxRateIndex;
}
#endif /* NEW_RATE_ADAPT_SUPPORT */


INT rtmp_get_rate_from_rate_tb(UCHAR *table, INT idx, RTMP_TX_RATE *tx_rate)
{
#ifdef NEW_RATE_ADAPT_SUPPORT
	if (ADAPT_RATE_TABLE(table)) {
		RTMP_RA_GRP_TB *rate_entry;

		rate_entry = PTX_RA_GRP_ENTRY(table, idx);
		tx_rate->mode = rate_entry->Mode;
		tx_rate->bw = rate_entry->BW;
		tx_rate->mcs = rate_entry->CurrMCS;
		tx_rate->sgi = rate_entry->ShortGI;
		tx_rate->stbc = rate_entry->STBC;
#ifdef DOT11_VHT_AC
		if (table == RateTableVht1S || table == RateTableVht2S || 
					table == RateTableVht2S_BW40 || 
					table == RateTableVht2S_BW20 || table == RateTableVht1S_MCS9
					|| (table == RateTableVht2S_MCS7))
			tx_rate->nss = rate_entry->dataRate;
		else
#endif /* DOT11_VHT_AC */
			tx_rate->nss = 0;
	}
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
	{
		RTMP_RA_LEGACY_TB *rate_entry;

		rate_entry = PTX_RA_LEGACY_ENTRY(table, idx);
		tx_rate->mode = rate_entry->Mode;
		tx_rate->bw = rate_entry->BW;
		tx_rate->mcs = rate_entry->CurrMCS;
		tx_rate->sgi = rate_entry->ShortGI;
		tx_rate->stbc = rate_entry->STBC;
		tx_rate->nss = 0;
	}

	return TRUE;
}


/*
	MlmeNewTxRate - called when a new TX rate was selected. Sets TX PHY to
		rate selected by pEntry->CurrTxRateIndex in pTable;
*/
VOID MlmeNewTxRate(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	RTMP_RA_LEGACY_TB *pNextTxRate;
	UCHAR *pTable;

	if ((pEntry == NULL) || (pEntry->pTable == NULL))
		return;
	else
		pTable = pEntry->pTable;
    
	if(pAd->LowerMcsValue || pAd->HigherMcsValue) {
	    CHAR Rssi = 0;
	    Rssi = RTMPMaxRssi(pAd, pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1], pEntry->RssiSample.AvgRssi[2]);
	    DBGPRINT(RT_DEBUG_TRACE,("#*#*#*#[%s] Rssi:%d CurrTxRateIndex:%d lastRateIdx:%d\n", 
			__FUNCTION__, Rssi, pEntry->CurrTxRateIndex, pEntry->lastRateIdx));
#ifdef NEW_RATE_ADAPT_SUPPORT
	    if (ADAPT_RATE_TABLE(pTable))
		    pNextTxRate = (RTMP_RA_LEGACY_TB *)PTX_RA_GRP_ENTRY(pTable, pEntry->CurrTxRateIndex);
	    else
#endif /* NEW_RATE_ADAPT_SUPPORT */
		pNextTxRate = PTX_RA_LEGACY_ENTRY(pTable, pEntry->CurrTxRateIndex);
 	    if(pAd->LowerMcsValue)
	    {
	        if( pNextTxRate && pNextTxRate->CurrMCS < pAd->LowerMcsValue) {
		        DBGPRINT(RT_DEBUG_TRACE,("#*#*1#*#[%s] CurrMCS:%d thus CurrTxRateIndex:%d is set to LowerMcsValue:%d \n",
                        __FUNCTION__, pNextTxRate->CurrMCS, pEntry->CurrTxRateIndex, pAd->LowerMcsValue));	    
		        pEntry->CurrTxRateIndex = pAd->LowerMcsValue;
                pEntry->HTPhyMode.field.MCS = pAd->LowerMcsValue;
	        }
	        if(pEntry->lastRateIdx < pAd->LowerMcsValue)
		        pEntry->lastRateIdx = pAd->LowerMcsValue;
	    }
	    if(pAd->HigherMcsValue)
	    {
		    if( pNextTxRate && pNextTxRate->CurrMCS > pAd->HigherMcsValue) {
		        DBGPRINT(RT_DEBUG_TRACE,("#*#*2#*#[%s] CurrMCS:%d thus CurrTxRateIndex:%d is set to HigherMcsValue:%d \n",
                        __FUNCTION__, pNextTxRate->CurrMCS, pEntry->CurrTxRateIndex, pAd->HigherMcsValue));
                pEntry->CurrTxRateIndex = pAd->HigherMcsValue;
                pEntry->HTPhyMode.field.MCS = pAd->HigherMcsValue;
            }
	    }
	    DBGPRINT(RT_DEBUG_TRACE,("#*#*MODIFIED#*#[%s] CurrTxRateIndex: %d lastRateIdx:%d \n", 
			__FUNCTION__, pEntry->CurrTxRateIndex, pEntry->lastRateIdx));
	}
	/*  Get pointer to CurrTxRate entry */
#ifdef NEW_RATE_ADAPT_SUPPORT
	if (ADAPT_RATE_TABLE(pTable))
		pNextTxRate = (RTMP_RA_LEGACY_TB *)PTX_RA_GRP_ENTRY(pTable, pEntry->CurrTxRateIndex);
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
		pNextTxRate = PTX_RA_LEGACY_ENTRY(pTable, pEntry->CurrTxRateIndex);

#ifdef NEW_RATE_ADAPT_SUPPORT
#ifdef WAPI_SUPPORT
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7636(pAd))
	{
		if ((pEntry->AuthMode == Ndis802_11AuthModeWAICERT) || (pEntry->AuthMode == Ndis802_11AuthModeWAIPSK))
		{
#ifdef MULTI_CLIENT_SUPPORT
			if ((pTable == RateSwitchTableAdapt11N2S || (pTable == RateSwitchTableAdapt11N2SForMultiClients)))
#else
			if (pTable == RateSwitchTableAdapt11N2S)
#endif
			{
				if ((pEntry->CurrTxRateIndex >= 14) && (pEntry->CurrTxRateIndex <= 16))
				{
					  pNextTxRate = (RTMP_RA_LEGACY_TB *)PTX_RA_GRP_ENTRY(pTable, 13);
				}
			}
		}
	}
#endif /* WAPI_SUPPORT */
#endif /* NEW_RATE_ADAPT_SUPPORT */

	/*  Set new rate */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		APMlmeSetTxRate(pAd, pEntry, pNextTxRate);
	}
#endif /*  CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
	/*  Disable invalid HT Duplicate modes to prevent PHY error */
	if (pEntry->HTPhyMode.field.MCS==32)
	{
		if ((pEntry->HTPhyMode.field.BW!=BW_40) && (pEntry->HTPhyMode.field.BW!=BW_80))
			pEntry->HTPhyMode.field.MCS = 0;
		else
			pEntry->HTPhyMode.field.STBC = 0;
	}
#endif /*  DOT11_N_SUPPORT */


	pAd->LastTxRate = (USHORT)(pEntry->HTPhyMode.word);

#ifdef STREAM_MODE_SUPPORT
	/*  Enable/disable stream mode based on MCS */
	if (pAd->CommonCfg.StreamMode!=0 &&
		pEntry->StreamModeMACReg!=0)
	{
		UINT streamWord;
		BOOLEAN mcsDisable;

		/* OFDM: depends on StreamModeMCS, CCK: always applies stream-mode */
		mcsDisable = (pEntry->HTPhyMode.field.MCS < 16) &&
				(pAd->CommonCfg.StreamModeMCS & (1<<pEntry->HTPhyMode.field.MCS))==0 &&
				(pEntry->HTPhyMode.field.MODE != MODE_CCK);

		streamWord = mcsDisable ? 0 : StreamModeRegVal(pAd);

		/*  Update Stream Mode control reg */
		RTMP_IO_WRITE32(pAd, pEntry->StreamModeMACReg+4, streamWord | (ULONG)(pEntry->Addr[4]) | (ULONG)(pEntry->Addr[5] << 8));
	}
#endif /* STREAM_MODE_SUPPORT */
}


VOID RTMPSetSupportMCS(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR OpMode,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
#ifdef DOT11_VHT_AC
	IN UCHAR vht_cap_len,
	IN VHT_CAP_IE *vht_cap,
#endif /* DOT11_VHT_AC */
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen)
{
	UCHAR idx, SupportedRatesLen = 0;
	UCHAR SupportedRates[MAX_LEN_OF_SUPPORTED_RATES];

	if (SupRateLen > 0)
	{
		if (SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES)
		{
			NdisMoveMemory(SupportedRates, SupRate, SupRateLen);
			SupportedRatesLen = SupRateLen;
		}
		else
		{
			UCHAR RateDefault[8] = {0x82, 0x84, 0x8b, 0x96, 0x12, 0x24, 0x48, 0x6c};
			
			NdisMoveMemory(SupportedRates, RateDefault, 8);
			SupportedRatesLen = 8;
			
			DBGPRINT(RT_DEBUG_TRACE,("%s():wrong SUPP RATES., Len=%d\n",
							__FUNCTION__, SupRateLen));
		}
	}

	if (ExtRateLen > 0)
	{
		if ((SupRateLen + ExtRateLen) <= MAX_LEN_OF_SUPPORTED_RATES)
		{
			NdisMoveMemory(&SupportedRates[SupRateLen], ExtRate, ExtRateLen);
			SupportedRatesLen += ExtRateLen;
		}
		else
		{
			NdisMoveMemory(&SupportedRates[SupRateLen], ExtRate, MAX_LEN_OF_SUPPORTED_RATES - ExtRateLen);
			SupportedRatesLen = MAX_LEN_OF_SUPPORTED_RATES;

		}
	}

	/* Clear Supported MCS Table */
	NdisZeroMemory(pEntry->SupportCCKMCS, MAX_LEN_OF_CCK_RATES);
	NdisZeroMemory(pEntry->SupportOFDMMCS, MAX_LEN_OF_OFDM_RATES);
	NdisZeroMemory(pEntry->SupportHTMCS, MAX_LEN_OF_HT_RATES);
#ifdef DOT11_VHT_AC
	NdisZeroMemory(pEntry->SupportVHTMCS, MAX_LEN_OF_VHT_RATES);
#endif /* DOT11_VHT_AC */

	pEntry->SupportRateMode = 0;

	for(idx = 0; idx < SupportedRatesLen; idx ++)
	{
		switch((SupportedRates[idx] & 0x7F)*5)
		{
			case 10:
				pEntry->SupportCCKMCS[MCS_0] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
				break;

			case 20:
				pEntry->SupportCCKMCS[MCS_1] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
				break;

			case 55:
				pEntry->SupportCCKMCS[MCS_2] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
				break;

			case 110:
				pEntry->SupportCCKMCS[MCS_3] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
				break;

			case 60:
				pEntry->SupportOFDMMCS[MCS_0] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 90:
				pEntry->SupportOFDMMCS[MCS_1] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 120:
				pEntry->SupportOFDMMCS[MCS_2] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 180:
				pEntry->SupportOFDMMCS[MCS_3] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 240:
				pEntry->SupportOFDMMCS[MCS_4] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 360:
				pEntry->SupportOFDMMCS[MCS_5] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 480:
				pEntry->SupportOFDMMCS[MCS_6] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 540:
				pEntry->SupportOFDMMCS[MCS_7] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;
		}	
	}
	
#ifdef GN_ONLY_AP_SUPPORT
        if ((OpMode == OPMODE_STA) || (OpMode == OPMODE_AP))
        {
     	    UINT32  mac_val,bar_rate_mask = 0xfff00000 ;
            RTMP_IO_READ32(pAd,AGG_ACR,&mac_val); // BAR rate
            mac_val &= (~ 0xfff00000);
            if(!(pEntry->SupportRateMode & SUPPORT_CCK_MODE) || !(pAd->CommonCfg.PhyMode & WMODE_B ))
            {
        	pAd->CommonCfg.bExcludeBRate = TRUE;
		mac_val &= (~ bar_rate_mask);
		mac_val |= (0x04b << 20);
            }else {
		pAd->CommonCfg.bExcludeBRate = FALSE;
		mac_val &= (~ bar_rate_mask);
	    }
            RTMP_IO_WRITE32(pAd,AGG_ACR, mac_val);		
        }
#endif /* GN_ONLY_AP_SUPPORT */
	if (HtCapabilityLen)
	{
		RT_PHY_INFO *pDesired_ht_phy = NULL;
		UCHAR j, bitmask;
		CHAR i;


#ifdef CONFIG_AP_SUPPORT
		if (OpMode == OPMODE_AP)
		{
#ifdef WDS_SUPPORT
			if (IS_ENTRY_WDS(pEntry))
				pDesired_ht_phy = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev.DesiredHtPhyInfo;
			else
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_APCLI(pEntry))
				pDesired_ht_phy = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.DesiredHtPhyInfo;
			else
#endif /* APCLI_SUPPORT */
				pDesired_ht_phy = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.DesiredHtPhyInfo;
		}
#endif /* CONFIG_AP_SUPPORT */

		if (pDesired_ht_phy == NULL)
			return;

		for (i = 23; i >= 0; i--)
		{
			j = i / 8;
			bitmask = (1 << (i - (j * 8)));

			if ((pDesired_ht_phy->MCSSet[j] & bitmask)
				&& (pHtCapability->MCSSet[j] & bitmask))
			{
				pEntry->SupportHTMCS[i] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_HT_MODE;
			}
		}

#ifdef DOT11_VHT_AC
		if ((vht_cap_len > 0)&& (vht_cap != NULL) && pDesired_ht_phy->bVhtEnable)
		{
			/* Currently we only support for MCS0~MCS7, so don't check mcs_map */
			NdisZeroMemory(&pEntry->SupportVHTMCS[0], sizeof(pEntry->SupportVHTMCS));
			switch (pAd->CommonCfg.TxStream)
			{
				case 2:
					if (vht_cap->mcs_set.rx_mcs_map.mcs_ss2 < VHT_MCS_CAP_NA)
					{
						for (i = 10; i <= 17; i++)
							pEntry->SupportVHTMCS[i] = TRUE;

						if (vht_cap->mcs_set.rx_mcs_map.mcs_ss2 == VHT_MCS_CAP_8) {
							pEntry->SupportVHTMCS[18] = TRUE;
						} else if (vht_cap->mcs_set.rx_mcs_map.mcs_ss2 == VHT_MCS_CAP_9) {
							pEntry->SupportVHTMCS[18] = TRUE;
							pEntry->SupportVHTMCS[19] = TRUE;
						}

						pEntry->SupportRateMode |= SUPPORT_VHT_MODE;
					}
				case 1:
					if (vht_cap->mcs_set.rx_mcs_map.mcs_ss1 < VHT_MCS_CAP_NA)
					{
						for (i = 0; i <= 7; i++)
							pEntry->SupportVHTMCS[i] = TRUE;

						if (vht_cap->mcs_set.rx_mcs_map.mcs_ss1 == VHT_MCS_CAP_8) {
							pEntry->SupportVHTMCS[8] = TRUE;
						} else if (vht_cap->mcs_set.rx_mcs_map.mcs_ss1 == VHT_MCS_CAP_9) {
							pEntry->SupportVHTMCS[8] = TRUE;
							pEntry->SupportVHTMCS[9] = TRUE;
						}

						pEntry->SupportRateMode |= SUPPORT_VHT_MODE;
					}
				default:
					break;
			}
		}
#endif /* DOT11_VHT_AC */
	}
}


INT	Set_RateAlg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ra_alg;

	ra_alg = simple_strtol(arg, 0, 10);

	if ((ra_alg < RATE_ALG_MAX_NUM) && (ra_alg != pAd->rateAlg))
	{
		UINT32 IdEntry;

		pAd->rateAlg = ra_alg;
		for(IdEntry = 0; IdEntry < MAX_LEN_OF_MAC_TABLE; IdEntry++)
			pAd->MacTab.Content[IdEntry].rateAlg = ra_alg;
	}

	DBGPRINT(RT_DEBUG_ERROR, ("%s: Set Alg = %d\n", __FUNCTION__, ra_alg));
	return TRUE;
}

