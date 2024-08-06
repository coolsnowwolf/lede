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

#ifndef _LINUX_R8126_PTP_H
#define _LINUX_R8126_PTP_H

#include <linux/ktime.h>
#include <linux/timecounter.h>
#include <linux/net_tstamp.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/ptp_classify.h>

struct rtl8126_ptp_info {
        s64 time_sec;
        u32 time_ns;
        u16 ts_info;
};

#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC
struct timespec {
        __kernel_old_time_t tv_sec;     /* seconds */
        long            tv_nsec;    /* nanoseconds */
};
#endif

enum PTP_CMD_TYPE {
        PTP_CMD_SET_LOCAL_TIME = 0,
        PTP_CMD_DRIFT_LOCAL_TIME,
        PTP_CMD_LATCHED_LOCAL_TIME,
};


struct rtl8126_private;
struct RxDescV3;

int rtl8126_get_ts_info(struct net_device *netdev,
                        struct ethtool_ts_info *info);

void rtl8126_ptp_reset(struct rtl8126_private *tp);
void rtl8126_ptp_init(struct rtl8126_private *tp);
void rtl8126_ptp_suspend(struct rtl8126_private *tp);
void rtl8126_ptp_stop(struct rtl8126_private *tp);

int rtl8126_ptp_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd);

void rtl8126_rx_ptp_pktstamp(struct rtl8126_private *tp, struct sk_buff *skb,
                             struct RxDescV3 *descv3);

#endif /* _LINUX_R8126_PTP_H */
