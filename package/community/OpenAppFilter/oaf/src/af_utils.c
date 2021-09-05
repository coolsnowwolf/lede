#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/version.h>
#include "af_utils.h"
u_int32_t af_get_timestamp_sec(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
	struct timespec64 ts;
	ktime_get_real_ts64(&ts);
	return (u_int32_t)ts.tv_sec;
#else
	struct timespec ts;
	ts = current_kernel_time();
	return ts.tv_sec;
#endif

}

int check_local_network_ip(unsigned int ip)
{
	if ((ip & 0xffff0000) == 0xc0a80000)
		return 1;
	else if ((ip & 0xfff00000) == 0xac100000)
		return 1;
	else if ((ip & 0xff000000) == 0x0a000000)
		return 1;
	else
		return 0;
}

void dump_str(char *name, unsigned char *p, int len)
{
	#define MAX_DUMP_STR_LEN 64
	char buf[MAX_DUMP_STR_LEN] = {0};
	if (len > MAX_DUMP_STR_LEN) {
		len = MAX_DUMP_STR_LEN - 1;
	}
	printk("%s: ",name);
	strncpy(buf, p, len);
	printk("[%s]\n", buf);
}

void dump_hex(char *name, unsigned char *p, int len)
{
	#define MAX_DUMP_STR_LEN 64
	int i;
	if (len > MAX_DUMP_STR_LEN) {
		len = MAX_DUMP_STR_LEN - 1;
	}
	printk("%s: ",name);
	for (i = 0; i < len; i++) {
		if (i % 16 == 0)
			printk("\n");
		printk("%02X ",*(p + i));
	}
	printk("\n");
}

#ifndef va_arg
typedef signed int acpi_native_int;
#ifndef _VALIST
#define _VALIST
typedef char *va_list;
#endif	

#define  _AUPBND                (sizeof (acpi_native_int) - 1)
#define  _ADNBND                (sizeof (acpi_native_int) - 1)


#define _bnd(X, bnd)            (((sizeof (X)) + (bnd)) & (~(bnd)))
#define va_arg(ap, T)           (*(T *)(((ap) += (_bnd (T, _AUPBND))) - (_bnd (T,_ADNBND))))
#define va_end(ap)              (void) 0
#define va_start(ap, A)         (void) ((ap) = (((char *) &(A)) + (_bnd (A,_AUPBND))))

#endif			

#ifndef TOLOWER
#define TOLOWER(x) ((x) | 0x20)
#endif


static long long k_simple_strtoll(const char *cp, char **endp, unsigned int base)
{
	if (*cp == '-')
		return -simple_strtoull(cp + 1, endp, base);

	return simple_strtoull(cp, endp, base);
}

static int skip_atoi(const char **s)
{
	int i=0;
	while (isdigit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}

char *skip_spaces(const char *str)
{
	while (isspace(*str) && ((unsigned char )*str != 0xa0))
		++str;
	return (char *)str;
}
static int k_vsscanf(const char *buf, const char *fmt, va_list args)
{
	const char *str = buf;
	char *next;
	char digit;
	int num = 0;
	u8 qualifier;
	u8 base;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,22)
	int field_width;
#else 
	s16 field_width;
#endif
	bool is_sign;
	while (*fmt && *str) {
		if (isspace(*fmt)) {
			fmt = skip_spaces(++fmt);
			str = skip_spaces(str);
		}

		if (*fmt != '%' && *fmt) {
			if (*fmt++ != *str++)
				break;
			continue;
		}
		if (!*fmt)
			break;
		++fmt;

		if (*fmt == '*') {
			while (!isspace(*fmt) && *fmt != '%' && *fmt)
				fmt++;
			while (!isspace(*str) && *str)
				str++;
			continue;
		}
		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);
		qualifier = -1;
		if (*fmt == 'h' || TOLOWER(*fmt) == 'l' ||
		    TOLOWER(*fmt) == 'z') {
			qualifier = *fmt++;
			if (unlikely(qualifier == *fmt)) {
				if (qualifier == 'h') {
					qualifier = 'H';
					fmt++;
				} else if (qualifier == 'l') {
					qualifier = 'L';
					fmt++;
				}
			}
		}
		if (!*fmt || !*str)
			break;
		base = 10;
		is_sign = 0;
		switch (*fmt++) {
		case 'c':
		{
			char *s = (char *)va_arg(args, char*);
			if (field_width == -1)
				field_width = 1;
			do {
				*s++ = *str++;
			} while (--field_width > 0 && *str);
			num++;
		}
		continue;
		case 's':
		{
			char *s = (char *)va_arg(args, char *);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,22)
			if(field_width == -1)
				field_width = INT_MAX;
#else
			if (field_width == -1)
				field_width = SHRT_MAX;
#endif
			str = skip_spaces(str);

			while (*str && (!isspace(*str)  || ((unsigned char )*str == 0xA0) )&& field_width--)
				*s++ = *str++;
			*s = '\0';
			num++;
		}
		continue;
		case 'n':
		{
			int *i = (int *)va_arg(args, int*);
			*i = str - buf;
		}
		continue;
		case 'o':
			base = 8;
			break;
		case 'x':
		case 'X':
			base = 16;
			break;
		case 'i':
			base = 0;
		case 'd':
			is_sign = 1;
		case 'u':
			break;
		case '%':
			if (*str++ != '%')
				return num;
			continue;
		default:
			return num;
		}
		str = skip_spaces(str);
		digit = *str;
		if (is_sign && digit == '-')
			digit = *(str + 1);
		if (!digit
		    || (base == 16 && !isxdigit(digit))
		    || (base == 10 && !isdigit(digit))
		    || (base == 8 && (!isdigit(digit) || digit > '7'))
		    || (base == 0 && !isdigit(digit)))
			break;

		switch (qualifier) {
		case 'H':	
			if (is_sign) {
				signed char *s = (signed char *)va_arg(args, signed char *);
				*s = (signed char)simple_strtol(str, &next, base);
			} else {
				unsigned char *s = (unsigned char *)va_arg(args, unsigned char *);
				*s = (unsigned char)simple_strtoul(str, &next, base);
			}
			break;
		case 'h':
			if (is_sign) {
				short *s = (short *)va_arg(args, short *);
				*s = (short)simple_strtol(str, &next, base);
			} else {
				unsigned short *s = (unsigned short *)va_arg(args, unsigned short *);
				*s = (unsigned short)simple_strtoul(str, &next, base);
			}
			break;
		case 'l':
			if (is_sign) {
				long *l = (long *)va_arg(args, long *);
				*l = simple_strtol(str, &next, base);
			} else {
				unsigned long *l = (unsigned long *)va_arg(args, unsigned long *);
				*l = simple_strtoul(str, &next, base);
			}
			break;
		case 'L':
			if (is_sign) {
				long long *l = (long long *)va_arg(args, long long *);
				*l = k_simple_strtoll(str, &next, base);
			} else {
				unsigned long long *l = (unsigned long long *)va_arg(args, unsigned long long *);
				*l = simple_strtoull(str, &next, base);
			}
			break;
		case 'Z':
		case 'z':
		{
			size_t *s = (size_t *)va_arg(args, size_t *);
			*s = (size_t)simple_strtoul(str, &next, base);
		}
		break;
		default:
			if (is_sign) {
				int *i = (int *)va_arg(args, int *);
				*i = (int)simple_strtol(str, &next, base);
			} else {
				unsigned int *i = (unsigned int *)va_arg(args, unsigned int*);
				*i = (unsigned int)simple_strtoul(str, &next, base);
			}
			break;
		}
		num++;
		if (!next)
			break;
		str = next;
	}
	if (*fmt == '%' && *(fmt + 1) == 'n') {
		int *p = (int *)va_arg(args, int *);
		*p = str - buf;
	}
	return num;
}


int k_sscanf(const char *buf, const char *fmt, ...)
{
	va_list args;
	int i;
	va_start(args, fmt);
	i = k_vsscanf(buf, fmt, args);
	va_end(args);
	return i;
}


