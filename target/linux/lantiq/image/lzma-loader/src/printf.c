/*
 * Copyright (C) 2001 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include	"printf.h"

extern void board_putc(char ch);

/* this is the maximum width for a variable */
#define		LP_MAX_BUF	256

/* macros */
#define		IsDigit(x)	( ((x) >= '0') && ((x) <= '9') )
#define		Ctod(x)		( (x) - '0')

/* forward declaration */
static int PrintChar(char *, char, int, int);
static int PrintString(char *, char *, int, int);
static int PrintNum(char *, unsigned long, int, int, int, int, char, int);

/* private variable */
static const char theFatalMsg[] = "fatal error in lp_Print!";

/* -*-
 * A low level printf() function.
 */
static void
lp_Print(void (*output)(void *, char *, int),
	 void * arg,
	 char *fmt,
	 va_list ap)
{

#define 	OUTPUT(arg, s, l)  \
  { if (((l) < 0) || ((l) > LP_MAX_BUF)) { \
       (*output)(arg, (char*)theFatalMsg, sizeof(theFatalMsg)-1); for(;;); \
    } else { \
      (*output)(arg, s, l); \
    } \
  }

    char buf[LP_MAX_BUF];

    char c;
    char *s;
    long int num;

    int longFlag;
    int negFlag;
    int width;
    int prec;
    int ladjust;
    char padc;

    int length;

    for(;;) {
	{
	    /* scan for the next '%' */
	    char *fmtStart = fmt;
	    while ( (*fmt != '\0') && (*fmt != '%')) {
		fmt ++;
	    }

	    /* flush the string found so far */
	    OUTPUT(arg, fmtStart, fmt-fmtStart);

	    /* are we hitting the end? */
	    if (*fmt == '\0') break;
	}

	/* we found a '%' */
	fmt ++;

	/* check for long */
	if (*fmt == 'l') {
	    longFlag = 1;
	    fmt ++;
	} else {
	    longFlag = 0;
	}

	/* check for other prefixes */
	width = 0;
	prec = -1;
	ladjust = 0;
	padc = ' ';

	if (*fmt == '-') {
	    ladjust = 1;
	    fmt ++;
	}

	if (*fmt == '0') {
	    padc = '0';
	    fmt++;
	}

	if (IsDigit(*fmt)) {
	    while (IsDigit(*fmt)) {
		width = 10 * width + Ctod(*fmt++);
	    }
	}

	if (*fmt == '.') {
	    fmt ++;
	    if (IsDigit(*fmt)) {
		prec = 0;
		while (IsDigit(*fmt)) {
		    prec = prec*10 + Ctod(*fmt++);
		}
	    }
	}


	/* check format flag */
	negFlag = 0;
	switch (*fmt) {
	 case 'b':
	    if (longFlag) {
		num = va_arg(ap, long int);
	    } else {
		num = va_arg(ap, int);
	    }
	    length = PrintNum(buf, num, 2, 0, width, ladjust, padc, 0);
	    OUTPUT(arg, buf, length);
	    break;

	 case 'd':
	 case 'D':
	    if (longFlag) {
		num = va_arg(ap, long int);
	    } else {
		num = va_arg(ap, int);
	    }
	    if (num < 0) {
		num = - num;
		negFlag = 1;
	    }
	    length = PrintNum(buf, num, 10, negFlag, width, ladjust, padc, 0);
	    OUTPUT(arg, buf, length);
	    break;

	 case 'o':
	 case 'O':
	    if (longFlag) {
		num = va_arg(ap, long int);
	    } else {
		num = va_arg(ap, int);
	    }
	    length = PrintNum(buf, num, 8, 0, width, ladjust, padc, 0);
	    OUTPUT(arg, buf, length);
	    break;

	 case 'u':
	 case 'U':
	    if (longFlag) {
		num = va_arg(ap, long int);
	    } else {
		num = va_arg(ap, int);
	    }
	    length = PrintNum(buf, num, 10, 0, width, ladjust, padc, 0);
	    OUTPUT(arg, buf, length);
	    break;

	 case 'x':
	    if (longFlag) {
		num = va_arg(ap, long int);
	    } else {
		num = va_arg(ap, int);
	    }
	    length = PrintNum(buf, num, 16, 0, width, ladjust, padc, 0);
	    OUTPUT(arg, buf, length);
	    break;

	 case 'X':
	    if (longFlag) {
		num = va_arg(ap, long int);
	    } else {
		num = va_arg(ap, int);
	    }
	    length = PrintNum(buf, num, 16, 0, width, ladjust, padc, 1);
	    OUTPUT(arg, buf, length);
	    break;

	 case 'c':
	    c = (char)va_arg(ap, int);
	    length = PrintChar(buf, c, width, ladjust);
	    OUTPUT(arg, buf, length);
	    break;

	 case 's':
	    s = (char*)va_arg(ap, char *);
	    length = PrintString(buf, s, width, ladjust);
	    OUTPUT(arg, buf, length);
	    break;

	 case '\0':
	    fmt --;
	    break;

	 default:
	    /* output this char as it is */
	    OUTPUT(arg, fmt, 1);
	}	/* switch (*fmt) */

	fmt ++;
    }		/* for(;;) */

    /* special termination call */
    OUTPUT(arg, "\0", 1);
}


/* --------------- local help functions --------------------- */
static int
PrintChar(char * buf, char c, int length, int ladjust)
{
    int i;

    if (length < 1) length = 1;
    if (ladjust) {
	*buf = c;
	for (i=1; i< length; i++) buf[i] = ' ';
    } else {
	for (i=0; i< length-1; i++) buf[i] = ' ';
	buf[length - 1] = c;
    }
    return length;
}

static int
PrintString(char * buf, char* s, int length, int ladjust)
{
    int i;
    int len=0;
    char* s1 = s;
    while (*s1++) len++;
    if (length < len) length = len;

    if (ladjust) {
	for (i=0; i< len; i++) buf[i] = s[i];
	for (i=len; i< length; i++) buf[i] = ' ';
    } else {
	for (i=0; i< length-len; i++) buf[i] = ' ';
	for (i=length-len; i < length; i++) buf[i] = s[i-length+len];
    }
    return length;
}

static int
PrintNum(char * buf, unsigned long u, int base, int negFlag,
	 int length, int ladjust, char padc, int upcase)
{
    /* algorithm :
     *  1. prints the number from left to right in reverse form.
     *  2. fill the remaining spaces with padc if length is longer than
     *     the actual length
     *     TRICKY : if left adjusted, no "0" padding.
     *		    if negtive, insert  "0" padding between "0" and number.
     *  3. if (!ladjust) we reverse the whole string including paddings
     *  4. otherwise we only reverse the actual string representing the num.
     */

    int actualLength =0;
    char *p = buf;
    int i;

    do {
	int tmp = u %base;
	if (tmp <= 9) {
	    *p++ = '0' + tmp;
	} else if (upcase) {
	    *p++ = 'A' + tmp - 10;
	} else {
	    *p++ = 'a' + tmp - 10;
	}
	u /= base;
    } while (u != 0);

    if (negFlag) {
	*p++ = '-';
    }

    /* figure out actual length and adjust the maximum length */
    actualLength = p - buf;
    if (length < actualLength) length = actualLength;

    /* add padding */
    if (ladjust) {
	padc = ' ';
    }
    if (negFlag && !ladjust && (padc == '0')) {
	for (i = actualLength-1; i< length-1; i++) buf[i] = padc;
	buf[length -1] = '-';
    } else {
	for (i = actualLength; i< length; i++) buf[i] = padc;
    }


    /* prepare to reverse the string */
    {
	int begin = 0;
	int end;
	if (ladjust) {
	    end = actualLength - 1;
	} else {
	    end = length -1;
	}

	while (end > begin) {
	    char tmp = buf[begin];
	    buf[begin] = buf[end];
	    buf[end] = tmp;
	    begin ++;
	    end --;
	}
    }

    /* adjust the string pointer */
    return length;
}

static void printf_output(void *arg, char *s, int l)
{
    int i;

    // special termination call
    if ((l==1) && (s[0] == '\0')) return;

    for (i=0; i< l; i++) {
	board_putc(s[i]);
	if (s[i] == '\n') board_putc('\r');
    }
}

void printf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    lp_Print(printf_output, 0, fmt, ap);
    va_end(ap);
}
