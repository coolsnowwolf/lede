/*
################################################################################
#
# r8125 is the Linux device driver released for Realtek 2.5Gigabit Ethernet
# controllers with PCI-Express interface.
#
# Copyright(c) 2020 Realtek Semiconductor Corp. All rights reserved.
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

//EEPROM opcodes
#define RTL_EEPROM_READ_OPCODE      06
#define RTL_EEPROM_WRITE_OPCODE     05
#define RTL_EEPROM_ERASE_OPCODE     07
#define RTL_EEPROM_EWEN_OPCODE      19
#define RTL_EEPROM_EWDS_OPCODE      16

#define RTL_CLOCK_RATE  3

void rtl8125_eeprom_type(struct rtl8125_private *tp);
void rtl8125_eeprom_cleanup(struct rtl8125_private *tp);
u16 rtl8125_eeprom_read_sc(struct rtl8125_private *tp, u16 reg);
void rtl8125_eeprom_write_sc(struct rtl8125_private *tp, u16 reg, u16 data);
void rtl8125_shift_out_bits(struct rtl8125_private *tp, int data, int count);
u16 rtl8125_shift_in_bits(struct rtl8125_private *tp);
void rtl8125_raise_clock(struct rtl8125_private *tp, u8 *x);
void rtl8125_lower_clock(struct rtl8125_private *tp, u8 *x);
void rtl8125_stand_by(struct rtl8125_private *tp);
void rtl8125_set_eeprom_sel_low(struct rtl8125_private *tp);
