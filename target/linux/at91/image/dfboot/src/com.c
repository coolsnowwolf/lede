/*----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support  -  ROUSSET  -
 *----------------------------------------------------------------------------
 * The software is delivered "AS IS" without warranty or condition of any
 * kind, either express, implied or statutory. This includes without
 * limitation any warranty or condition with respect to merchantability or
 * fitness for any particular purpose, or against the infringements of
 * intellectual property rights of others.
 *----------------------------------------------------------------------------
 * File Name           : com.c
 * Object              : 
 * Creation            : HIi   03/27/2003
 *
 *----------------------------------------------------------------------------
 */
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "config.h"
#include "com.h"
#include "stdio.h"

static char erase_seq[] = "\b \b";		/* erase sequence	*/

#define MAX_UARTS 1

//unsigned int usa[2] = {(unsigned int)AT91C_BASE_DBGU, (unsigned int)AT91C_ALTERNATE_USART};
unsigned int usa[1] = {(unsigned int)AT91C_BASE_DBGU};
unsigned int us;
int port_detected;

void at91_init_uarts(void)
{
	int i;

	port_detected = 0;
	AT91F_DBGU_CfgPIO();
	AT91F_US0_CfgPIO();
	AT91F_US0_CfgPMC();

	for(i=0; i<MAX_UARTS; i++) {
		us = usa[i];
		AT91F_US_ResetRx((AT91PS_USART)us);
		AT91F_US_ResetTx((AT91PS_USART)us);

		// Configure DBGU
		AT91F_US_Configure(
			(AT91PS_USART)us, // DBGU base address
			AT91C_MASTER_CLOCK,            // 60 MHz
			AT91C_US_ASYNC_MODE,           // mode Register to be programmed
			115200,                        // baudrate to be programmed
			0                              // timeguard to be programmed
			);

		// Enable Transmitter
		AT91F_US_EnableTx((AT91PS_USART)us);
		// Enable Receiver
		AT91F_US_EnableRx((AT91PS_USART)us);
	}
	us = usa[0];
}

int at91_serial_putc(int ch)
{
	if (ch == '\n')
		at91_serial_putc('\r');
	while (!AT91F_US_TxReady((AT91PS_USART)us));
	AT91F_US_PutChar((AT91PS_USART)us, (char)ch);
	return ch;
}

/* This getc is modified to be able work on more than one port. On certain
 * boards (i.e. Figment Designs VersaLink), the debug port is not available
 * once the unit is in it's enclosure, so, if one needs to get into dfboot
 * for any reason it is impossible. With this getc, it scans between the debug
 * port and another port and once it receives a character, it sets that port
 * as the debug port. */
int at91_serial_getc()
{
	while(1) {
#if 0
		if (!port_detected) {
			if (us == usa[0]) {
				us = usa[1];
			}
			else {
				us = usa[0];
			}
		}
#endif
		if(AT91F_US_RxReady((AT91PS_USART)us)) {
#if 0
			port_detected = 1;
#endif
			return((int)AT91F_US_GetChar((AT91PS_USART)us));
		}
	}
}

/*-----------------------------------------------------------------------------
 * Function Name       : AT91F_ReadLine()
 * Object              : 
 * Input Parameters    : 
 * Return value		: 
 *-----------------------------------------------------------------------------
 */
int AT91F_ReadLine (const char *const prompt, char *console_buffer)
{
	char *p = console_buffer;
	int	n = 0;					/* buffer index		*/
	int	plen = strlen (prompt);	/* prompt length	*/
	int	col;					/* output column cnt	*/
	char	c;

	/* print prompt */
	if (prompt)
		printf(prompt);
	col = plen;

	for (;;) {
		c = getc();

		switch (c) {
			case '\r':				/* Enter		*/
			case '\n':
				*p = '\0';
				puts ("\n");
				return (p - console_buffer);

			case 0x03:				/* ^C - break	*/
				console_buffer[0] = '\0';	/* discard input */
				return (-1);

			case 0x08:				/* ^H  - backspace	*/
			case 0x7F:				/* DEL - backspace	*/
				if (n) {
					--p;
					printf(erase_seq);
					col--;
					n--;
					}
				continue;

			default:
			/*
			 * Must be a normal character then
			 */
			if (n < (AT91C_CB_SIZE -2)) 
			{
				++col;		/* echo input		*/
				putc(c);
				*p++ = c;
				++n;
			} 
			else 
			{			/* Buffer full		*/
				putc('\a');
			}
		}
	}
}


/*-----------------------------------------------------------------------------
 * Function Name       : AT91F_WaitKeyPressed()
 * Object              : 
 * Input Parameters    : 
 * Return value		: 
 *-----------------------------------------------------------------------------
 */
void AT91F_WaitKeyPressed(void)
{
	int c;
    	puts("KEY");
		c = getc();
	putc('\n');
}

int puts(const char *str)
{
  while(*str != 0) {
		at91_serial_putc(*str);
		str++;
		}
	return 1;
}

int putc(int c)
{
  return at91_serial_putc(c);
}

int putchar(c)
{
	return putc(c);
}

int getc()
{
  return at91_serial_getc();
}

int strlen(const char *str)
{
  int len = 0;

  if(str == (char *)0)
    return 0;

  while(*str++ != 0)
    len++;

  return len;
}

#define ZEROPAD 1               /* pad with zero */
#define SIGN    2               /* unsigned/signed long */
#define LEFT    4              /* left justified */
#define LARGE   8              /* use 'ABCDEF' instead of 'abcdef' */

#define do_div(n,base) ({ \
        int __res; \
        __res = ((unsigned) n) % (unsigned) base; \
        n = ((unsigned) n) / (unsigned) base; \
        __res; \
})

static int number(int num, int base, int size,
                  int precision, int type)
{
  char c, sign, tmp[66];
  const char *digits="0123456789ABCDEF";
  int i;

  if (type & LEFT)
    type &= ~ZEROPAD;
  if (base < 2 || base > 16)
    return 0;
  c = (type & ZEROPAD) ? '0' : ' ';
  sign = 0;

  if(type & SIGN && num < 0)
    {
      sign = '-';
      num = -num;
      size--;
    }
  
  i = 0;
  if(num == 0)
    tmp[i++] = digits[0];
  else while(num != 0)
    tmp[i++] = digits[do_div(num, base)];

  if(i > precision)
    precision = i;
  size -= precision;
  
  if(!(type&(ZEROPAD+LEFT)))
    while(size-->0)
      putc(' ');
  
  if(sign)
    putc(sign);

  if (!(type & LEFT))
    while (size-- > 0)
      putc(c);

  while (i < precision--)
    putc('0');
  
  while (i-- > 0)
    putc(tmp[i]);

  while (size-- > 0)
    putc(' ');;

  return 1;
}

int hvfprintf(const char *fmt, va_list va)
{
  char *s;

	do {
		if(*fmt == '%')	{
			bool done = false;

			int type = 0;
			int precision = 0;

			do {
				fmt++;
				switch(*fmt) {
				case '0' :
					if(!precision)
						type |= ZEROPAD;
				case '1' :
				case '2' :
				case '3' :
				case '4' :
				case '5' :
				case '6' :
				case '7' :
				case '8' :
				case '9' :
					precision = precision * 10 + (*fmt - '0');
					break;
				case '.' :
					break;
				case 's' :
					s = va_arg(va, char *);
					if(!s)
						puts("<NULL>");
					else
						puts(s);
					done = true;
					break;
				case 'c' :
					putc(va_arg(va, int));
					done = true;
					break;
				case 'd' :
					number(va_arg(va, int), 10, 0, precision, type);
					done = true;
					break;
				case 'x' :
				case 'X' :
					number(va_arg(va, int), 16, 0, precision, type);
					done = true;
					break;
				case '%' :
					putc(*fmt);
					done = true;
				default: 
					putc('%');
					putc(*fmt);
					done = true;
					break;
				} 
			} while(!done);
		} else if(*fmt == '\\') {
			fmt++;
			if(*fmt == 'r') {
				putc('\r');
			} else if(*fmt == 'n') { 
				putc('\n');
			}
        	} else {
         		putc(*fmt);
        	}
		fmt++;
	} while(*fmt != 0);
  
  return 0;
}

int printf(const char *fmt, ...)
{
  va_list ap;
  int i;

  va_start(ap, fmt);
  i = hvfprintf(fmt, ap);
  va_end(ap);

  return i;
}
