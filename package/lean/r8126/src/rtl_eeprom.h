/* SPDX-License-Identifier: GPL-2.0-only */
/*
################################################################################
#
# r8126 is the Linux device driver released for Realtek 5 Gigabit Ethernet
# controllers with PCI-Express interface.
#
# Copyright(c) 2024 Realtek Semiconductor Corp. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, see <http://www.gnu.org/licenses/>.
#
# Author:
# Realtek NIC software team <nicfae@realtek.com>
# No. 2, Innovation Road II, Hsinchu Science Park, Hsinchu 300, Taiwan
#
################################################################################
*/

/************************************************************************************
 *  This product is covered by one or more of the following patents:
 *  US6,570,884, US6,115,776, and US6,327,625.
 ***********************************************************************************/

#ifndef _LINUX_RTLEEPROM_H
#define _LINUX_RTLEEPROM_H

//EEPROM opcodes
#define RTL_EEPROM_READ_OPCODE      06
#define RTL_EEPROM_WRITE_OPCODE     05
#define RTL_EEPROM_ERASE_OPCODE     07
#define RTL_EEPROM_EWEN_OPCODE      19
#define RTL_EEPROM_EWDS_OPCODE      16

#define RTL_CLOCK_RATE  3

void rtl8126_eeprom_type(struct rtl8126_private *tp);
void rtl8126_eeprom_cleanup(struct rtl8126_private *tp);
u16 rtl8126_eeprom_read_sc(struct rtl8126_private *tp, u16 reg);
void rtl8126_eeprom_write_sc(struct rtl8126_private *tp, u16 reg, u16 data);
void rtl8126_shift_out_bits(struct rtl8126_private *tp, int data, int count);
u16 rtl8126_shift_in_bits(struct rtl8126_private *tp);
void rtl8126_raise_clock(struct rtl8126_private *tp, u8 *x);
void rtl8126_lower_clock(struct rtl8126_private *tp, u8 *x);
void rtl8126_stand_by(struct rtl8126_private *tp);
void rtl8126_set_eeprom_sel_low(struct rtl8126_private *tp);

#endif /* _LINUX_RTLEEPROM_H */
