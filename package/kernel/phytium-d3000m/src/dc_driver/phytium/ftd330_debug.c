// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include <linux/time.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include "ftd330_debug.h"

#define WRITE_CMD kernel_write

static const char buf[] = "Write 32'h";
static const char buf2[] = " 32'h";
static const char buf3[] = "\n";

/* for capture dump */
static char log_file_name[256];

/* for capture dump */
static char *capture_path = "/root/";
module_param(capture_path, charp, 0);
MODULE_PARM_DESC(capture_path, "A string passed to the DC capture dump path");

struct _ftd330_debug_reg_value {
	u32 reg;
	u32 vaule;
};

static struct _ftd330_debug_reg_value reg_value_cache[512];
static u32 index;

int ftd330_debug_file_create(struct file **fp)
{
	struct timespec64 ts;
	ktime_t kt;
	struct tm tm_time;
	char time_str[13];
	int offset;

	if (!capture_path) {
		pr_err("Please specify the path to the capture dump.\n");
		return -1;
	}

	kt = ktime_get_real();
	ts = ktime_to_timespec64(kt);
	time64_to_tm(ts.tv_sec, 0, &tm_time);
	snprintf(time_str, sizeof(time_str), "%02d%02d%02d%02d", tm_time.tm_mday, tm_time.tm_hour,
		 tm_time.tm_min, tm_time.tm_sec);

	offset = snprintf(log_file_name, 256, "%s/ftd330_dc_capture_%s.log", capture_path, time_str);

	if (offset > 256) {
		pr_err("Buffer overflow when creating the phy debug file.\n");
		return -1;
	}

	*fp = filp_open(log_file_name, O_RDWR | O_CREAT, 0644);

	if (IS_ERR(*fp)) {
		pr_err("Failed to open the capture path: %s, ret:%d\n", log_file_name, -1);
		return -1;
	}

	return 0;
}

void ftd330_debug_file_close(struct file **fp)
{
	if (*fp)
		filp_close(*fp, NULL);
}

int ftd330_debug_reset(struct file **fp)
{
	ftd330_debug_file_close(fp);
	if (ftd330_debug_file_create(fp)) {
		pr_err("Failed to reset the phy debug file: ret:%d\n", -1);
		return -1;
	}

	return 0;
}

static void _reverse_str(char *source, char target[], uint length)
{
	uint i;

	for (i = 0; i < length; i++)
		target[i] = source[length - 1 - i];
	target[i] = 0;
}

static void _to_hex(uint num, char hex_str[])
{
	uint n = num;
	char hextable[] = "0123456789ABCDEF";
	char temphex[16], hex[16];
	uint i = 0;
	int index = 0;
	int j, k;

	while (n) {
		temphex[i++] = hextable[n % 16];
		n /= 16;
	}

	temphex[i] = 0;
	_reverse_str(temphex, hex, i);
	for (j = 0; j < 8 - i; ++j, index++)
		hex_str[j] = '0';

	for (k = 0; k < i; ++k)
		hex_str[index++] = hex[k];
	hex_str[index] = 0;
}

static void _flush_to_disk(struct file *fp, u32 reg, u32 value)
{
	char addr_str[9];
	char data_str[9];
	loff_t pos;

	_to_hex(reg, addr_str);
	_to_hex(value, data_str);

	if (!fp)
		return;

	pos = fp->f_pos;
	WRITE_CMD(fp, buf, sizeof(buf) - 1, &pos);
	fp->f_pos = pos;

	pos = fp->f_pos;
	WRITE_CMD(fp, addr_str, 8, &pos);
	fp->f_pos = pos;

	pos = fp->f_pos;
	WRITE_CMD(fp, buf2, sizeof(buf2) - 1, &pos);
	fp->f_pos = pos;

	pos = fp->f_pos;
	WRITE_CMD(fp, data_str, 8, &pos);
	fp->f_pos = pos;

	pos = fp->f_pos;
	WRITE_CMD(fp, buf3, sizeof(buf3) - 1, &pos);
	fp->f_pos = pos;
}

void ftd330_debug_dump_capture(struct file *fp, u32 reg, u32 value)
{
	u32 i;

	if (irqs_disabled()) {
		if (index < 512) {
			reg_value_cache[index].reg = reg;
			reg_value_cache[index].vaule = value;
			index++;
		} else {
			pr_err("Debug capture  dump out of cache:reg:%d, vaule:%d.\n", reg, value);
			return;
		}
	} else {
		for (i = 0; i < index; i++)
			_flush_to_disk(fp, reg_value_cache[i].reg, reg_value_cache[i].vaule);

		_flush_to_disk(fp, reg, value);
		index = 0;
	}

}
