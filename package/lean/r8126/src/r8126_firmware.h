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

#ifndef _LINUX_R8126_FIRMWARE_H
#define _LINUX_R8126_FIRMWARE_H

#include <linux/device.h>
#include <linux/firmware.h>

struct rtl8126_private;
typedef void (*rtl8126_fw_write_t)(struct rtl8126_private *tp, u16 reg, u16 val);
typedef u32 (*rtl8126_fw_read_t)(struct rtl8126_private *tp, u16 reg);

#define RTL8126_VER_SIZE		32

struct rtl8126_fw {
        rtl8126_fw_write_t phy_write;
        rtl8126_fw_read_t phy_read;
        rtl8126_fw_write_t mac_mcu_write;
        rtl8126_fw_read_t mac_mcu_read;
        const struct firmware *fw;
        const char *fw_name;
        struct device *dev;

        char version[RTL8126_VER_SIZE];

        struct rtl8126_fw_phy_action {
                __le32 *code;
                size_t size;
        } phy_action;
};

int rtl8126_fw_request_firmware(struct rtl8126_fw *rtl_fw);
void rtl8126_fw_release_firmware(struct rtl8126_fw *rtl_fw);
void rtl8126_fw_write_firmware(struct rtl8126_private *tp, struct rtl8126_fw *rtl_fw);

#endif /* _LINUX_R8126_FIRMWARE_H */
