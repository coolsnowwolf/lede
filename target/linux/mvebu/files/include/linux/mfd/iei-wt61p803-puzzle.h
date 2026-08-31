/* SPDX-License-Identifier: GPL-2.0-only */
/* IEI WT61P803 PUZZLE MCU Driver
 * System management microcontroller for fan control, temperature sensor reading,
 * LED control and system identification on IEI Puzzle series ARM-based appliances.
 *
 * Copyright (C) 2020 Sartura Ltd.
 * Author: Luka Kovacic <luka.kovacic@sartura.hr>
 */

#ifndef _MFD_IEI_WT61P803_PUZZLE_H_
#define _MFD_IEI_WT61P803_PUZZLE_H_

#define IEI_WT61P803_PUZZLE_BUF_SIZE 512

/* Command magic numbers */
#define IEI_WT61P803_PUZZLE_CMD_HEADER_START		0x40 /* @ */
#define IEI_WT61P803_PUZZLE_CMD_HEADER_START_OTHER	0x25 /* % */
#define IEI_WT61P803_PUZZLE_CMD_HEADER_EEPROM		0xF7

#define IEI_WT61P803_PUZZLE_CMD_RESPONSE_OK		0x30 /* 0 */
#define IEI_WT61P803_PUZZLE_CHECKSUM_RESPONSE_OK	0x70

#define IEI_WT61P803_PUZZLE_CMD_EEPROM_READ		0xA1
#define IEI_WT61P803_PUZZLE_CMD_EEPROM_WRITE		0xA0

#define IEI_WT61P803_PUZZLE_CMD_OTHER_VERSION		0x56 /* V */
#define IEI_WT61P803_PUZZLE_CMD_OTHER_BUILD		0x42 /* B */
#define IEI_WT61P803_PUZZLE_CMD_OTHER_BOOTLOADER_MODE	0x4D /* M */
#define IEI_WT61P803_PUZZLE_CMD_OTHER_MODE_BOOTLOADER	0x30
#define IEI_WT61P803_PUZZLE_CMD_OTHER_MODE_APPS		0x31
#define IEI_WT61P803_PUZZLE_CMD_OTHER_PROTOCOL_VERSION	0x50 /* P */

#define IEI_WT61P803_PUZZLE_CMD_FUNCTION_SINGLE		0x43 /* C */
#define IEI_WT61P803_PUZZLE_CMD_FUNCTION_OTHER		0x4F /* O */
#define IEI_WT61P803_PUZZLE_CMD_FUNCTION_OTHER_STATUS	0x53 /* S */
#define IEI_WT61P803_PUZZLE_CMD_FUNCTION_OTHER_POWER_LOSS 0x41 /* A */

#define IEI_WT61P803_PUZZLE_CMD_LED			0x52 /* R */
#define IEI_WT61P803_PUZZLE_CMD_LED_SET(n)		(0x30 | (n))

#define IEI_WT61P803_PUZZLE_CMD_TEMP			0x54 /* T */
#define IEI_WT61P803_PUZZLE_CMD_TEMP_ALL		0x41 /* A */

#define IEI_WT61P803_PUZZLE_CMD_FAN			0x46 /* F */
#define IEI_WT61P803_PUZZLE_CMD_FAN_PWM_READ		0x5A /* Z */
#define IEI_WT61P803_PUZZLE_CMD_FAN_PWM_WRITE		0x57 /* W */
#define IEI_WT61P803_PUZZLE_CMD_FAN_PWM_BASE		0x30
#define IEI_WT61P803_PUZZLE_CMD_FAN_RPM_BASE		0x41 /* A */

#define IEI_WT61P803_PUZZLE_CMD_FAN_PWM(x) (IEI_WT61P803_PUZZLE_CMD_FAN_PWM_BASE + (x)) /* 0 - 1 */
#define IEI_WT61P803_PUZZLE_CMD_FAN_RPM(x) (IEI_WT61P803_PUZZLE_CMD_FAN_RPM_BASE + (x)) /* 0 - 5 */

struct iei_wt61p803_puzzle_mcu_version;
struct iei_wt61p803_puzzle_reply;
struct iei_wt61p803_puzzle;

int iei_wt61p803_puzzle_write_command_watchdog(struct iei_wt61p803_puzzle *mcu,
					       unsigned char *cmd, size_t size,
					       unsigned char *reply_data, size_t *reply_size,
					       int retry_count);

int iei_wt61p803_puzzle_write_command(struct iei_wt61p803_puzzle *mcu,
				      unsigned char *cmd, size_t size,
				      unsigned char *reply_data, size_t *reply_size);

#endif /* _MFD_IEI_WT61P803_PUZZLE_H_ */
