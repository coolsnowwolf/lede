#include <stdarg.h>
#include <stdbool.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

void at91_init_uarts(void);
int puts(const char *str);
int putc(int c);
int putchar(int c);
int getc();

int strlen(const char *str);

int hvfprintf(const char *fmt, va_list ap);

int printf(const char *fmt, ...);
