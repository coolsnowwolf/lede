/*
 * (C) Copyright 2012 Quantenna Communications Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __RUBY_PM_H
#define __RUBY_PM_H

/* Power save levels */
#define BOARD_PM_LEVEL_FORCE_NO			(0)
#define BOARD_PM_LEVEL_NO			(1)
#define BOARD_PM_LEVEL_SLOW_DOWN		(2)
#define BOARD_PM_LEVEL_LATENCY_UP		(3)
#define BOARD_PM_LEVEL_DISTANCE_DOWN		(4)
#define BOARD_PM_LEVEL_IDLE			(5)
#define BOARD_PM_LEVEL_SUSPEND			(6)

#define BOARD_PM_LEVEL_INIT			BOARD_PM_LEVEL_FORCE_NO

/* Duty level, shared between Lhost and MuC */
#define BOARD_PM_LEVEL_DUTY			BOARD_PM_LEVEL_IDLE

/* Names of power save governors */
#define BOARD_PM_GOVERNOR_WLAN			"wlan"
#define BOARD_PM_GOVERNOR_QDISC			"qdisc"
#define BOARD_PM_GOVERNOR_QCSAPI		"qcsapi"

/* wlan timings to switch between modes */
#define BOARD_PM_WLAN_IDLE_TIMEOUT		(120 * HZ)
#define BOARD_PM_WLAN_DEFAULT_TIMEOUT		(0)

/* qdisc parameters to switch between modes */
#define BOARD_PM_QDISC_TIMER_TIMEOUT		(50/*ms*/ * HZ / 1000)
#define BOARD_PM_QDISC_SPEEDUP_THRESHOLD	(400)
#define BOARD_PM_QDISC_SLOWDOWN_THRESHOLD	(100)
#define BOARD_PM_QDISC_SLOWDOWN_COUNT		(80)
#define BOARD_PM_QDISC_SLOWDOWN_TIMEOUT		(3 * HZ)
#define BOARD_PM_QDISC_DEFAULT_TIMEOUT		(0)

/* Beacon TSF setting */
#define BOARD_PM_WAKE_BEACON_TSF_DEADLINE_PCT		(50)
#define BOARD_PM_WAKE_BEACON_TSF_ALERT_PCT		(25)

/* Default setting, shared between Lhost and MuC */
#define BOARD_PM_PDUTY_PERIOD_MS_DEFAULT		80
#define BOARD_PM_PDUTY_PCT_LOW_DEFAULT			80
#define BOARD_PM_SUSPEND_PERIOD_MS_DEFAULT		100
#define BOARD_PM_SUSPEND_PCT_LOW_DEFAULT		99

/* Multiple Periods Support */
#define BOARD_PM_PERIOD_CHANGE_INTERVAL			1
#define BOARD_PM_PERIOD_CNT			3

enum qtn_pm_param {
	QTN_PM_CURRENT_LEVEL,

	QTN_PM_SUSPEND,
	QTN_PM_SUSPEND_PERIOD_MS,
	QTN_PM_SUSPEND_PCT_LOW,
	QTN_PM_SUSPEND_HRESET,
	QTN_PM_SUSPEND_ALLCHAINS_DISABLE,

	QTN_PM_PDUTY,
	QTN_PM_PDUTY_PERIOD_MS,
	QTN_PM_PDUTY_PCT_LOW,
	QTN_PM_PDUTY_HRESET,
	QTN_PM_PDUTY_RXCHAINS_DISABLE,

	QTN_PM_MUC_SLEEP,

	QTN_PM_RXCHAIN_IDLE_COUNT,
	QTN_PM_RXCHAIN_IDLE_LEVEL,
	QTN_PM_TXCHAIN_IDLE_COUNT,
	QTN_PM_TXCHAIN_IDLE_LEVEL,

	QTN_PM_PAUSE_MGMT_PROBERESP,
	QTN_PM_PAUSE_MGMT_ASSOCRESP,
	QTN_PM_PAUSE_MGMT_AUTH,

	/* For Multiple Periods Support */
	QTN_PM_PERIOD_CHANGE_INTERVAL,	/* How long period setting will be changed(unit: second) */
	QTN_PM_PERIOD_CNT,	/* How many periods in period group(Max 3) */
	QTN_PM_PERIOD_GROUP,	/* Period group(Max 3 periods, each <= 255ms, unit: millisecond)*/

	QTN_PM_IOCTL_MAX
};

#define QTN_PM_PARAM_NAMES	{						\
	"level",			/* QTN_PM_CURRENT_LEVEL */		\
	"suspend_level",		/* QTN_PM_SUSPEND */			\
	"suspend_period",		/* QTN_PM_SUSPEND_PERIOD_MS */		\
	"suspend_pct",			/* QTN_PM_SUSPEND_PCT_LOW */		\
	"suspend_hreset",		/* QTN_PM_SUSPEND_HRESET */		\
	"suspend_allchains",		/* QTN_PM_SUSPEND_ALLCHAINS_DISABLE */	\
	"pduty_level",			/* QTN_PM_PDUTY */			\
	"pduty_period",			/* QTN_PM_PDUTY_PERIOD_MS */		\
	"pduty_pct",			/* QTN_PM_PDUTY_PCT_LOW */		\
	"pduty_hreset",			/* QTN_PM_PDUTY_HRESET */		\
	"pduty_rxchains",		/* QTN_PM_PDUTY_RXCHAINS_DISABLE */	\
	"muc_sleep_level",		/* QTN_PM_MUC_SLEEP */			\
	"rxchain_count",		/* QTN_PM_RXCHAIN_IDLE_COUNT */		\
	"rxchain_level",		/* QTN_PM_RXCHAIN_IDLE_LEVEL */		\
	"txchain_count",		/* QTN_PM_TXCHAIN_IDLE_COUNT */		\
	"txchain_level",		/* QTN_PM_TXCHAIN_IDLE_LEVEL */		\
	"pause_proberesp",		/* QTN_PM_PAUSE_MGMT_PROBERESP */	\
	"pause_assocresp",		/* QTN_PM_PAUSE_MGMT_ASSOCRESP */	\
	"pause_auth",			/* QTN_PM_PAUSE_MGMT_ASSOCRESP */	\
	"period_change_interval",	/* QTN_PM_PERIOD_CHANGE_INTERVAL */	\
	"period_cnt",			/* QTN_PM_PERIOD_CNT */	\
	"period_group"			/* QTN_PM_PERIOD_GROUP */	\
}

#define QTN_PM_PARAM_DEFAULTS	{							\
	BOARD_PM_LEVEL_INIT,			/* QTN_PM_CURRENT_LEVEL */		\
	BOARD_PM_LEVEL_SUSPEND,			/* QTN_PM_SUSPEND */			\
	BOARD_PM_SUSPEND_PERIOD_MS_DEFAULT,	/* QTN_PM_SUSPEND_PERIOD_MS */		\
	BOARD_PM_SUSPEND_PCT_LOW_DEFAULT,	/* QTN_PM_SUSPEND_PCT_LOW */		\
	1,					/* QTN_PM_SUSPEND_HRESET */		\
	1,					/* QTN_PM_SUSPEND_ALL_CHAINS_DISABLE */	\
	BOARD_PM_LEVEL_DUTY,			/* QTN_PM_PDUTY */			\
	BOARD_PM_PDUTY_PERIOD_MS_DEFAULT,	/* QTN_PM_PDUTY_PERIOD_MS */		\
	BOARD_PM_PDUTY_PCT_LOW_DEFAULT,		/* QTN_PM_PDUTY_PCT_LOW */		\
	0,					/* QTN_PM_PDUTY_HRESET */		\
	1,					/* QTN_PM_PDUTY_RXCHAINS_DISABLE */	\
	BOARD_PM_LEVEL_LATENCY_UP,		/* QTN_PM_MUC_SLEEP */			\
	4,					/* QTN_PM_RXCHAIN_IDLE_COUNT */		\
	BOARD_PM_LEVEL_DISTANCE_DOWN,		/* QTN_PM_RXCHAIN_IDLE_LEVEL */		\
	4,					/* QTN_PM_TXCHAIN_IDLE_COUNT */		\
	BOARD_PM_LEVEL_DISTANCE_DOWN,		/* QTN_PM_TXCHAIN_IDLE_LEVEL */		\
	60000,					/* QTN_PM_PAUSE_MGMT_PROBERESP */	\
	5000,					/* QTN_PM_PAUSE_MGMT_ASSOCRESP */	\
	5000,					/* QTN_PM_PAUSE_MGMT_AUTH */		\
	BOARD_PM_PERIOD_CHANGE_INTERVAL,	/* QTN_PM_PERIOD_CHANGE_INTERVAL */	\
	BOARD_PM_PERIOD_CNT,			/* QTN_PM_PERIOD_CNT */	\
	0x50321E				/* QTN_PM_PERIOD_GROUP(30ms, 50ms, 80ms) */	\
}

#define QTN_PM_UNPACK_PARAM(x)		((x) & 0xFF)
#define QTN_PM_UNPACK_VALUE(x)		((x) >> 8)
#define QTN_PM_PACK_PARAM_VALUE(p, v)	(((p) & 0xFF) | (((v) << 8) & 0xFFFFFF00))

#endif /* __RUBY_PM_H */

