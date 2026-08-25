/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2020, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	epon_hwtest.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Taylor.Fang	2020/7/26		Create
*/

#ifndef _EPON_CMD_H_
#define _EPON_CMD_H_

int eponCmdInit(void);
void eponCmdExit(void);

void eponMacTableInit(void);
void eponMacTableExit(void);

static int doEponSetLlidThrshldNum(int argc, char *argv[], void *p);
static int doEponSetLlidQueThrshld(int argc, char *argv[], void *p);

void get_register_count(XPON_DEBUG_STATISTIC *tmpReg);
static void get_register_state(XPON_DEBUG_STATE *tmpReg);

void set_fix_reg_list(void);
static void add_fix_reg_list(__u32 addr, __u32 value);
static void show_fix_reg_list(void);
static void del_fix_reg_list(void);

static int doEponMpcp(int argc, char *argv[], void *p);
static int doEponSet(int argc, char *argv[], void *p);
static int doEponStatistic(int argc, char *argv[], void *p);
static int doEponClearStatistic(void);
static int doEponStaticRpt(int argc, char *argv[], void *p);
static int doEponState(int argc, char *argv[], void *p);
static int doEponReg(int argc, char *argv[], void *p);
static int doEponLsrTm(int argc, char *argv[], void *p);
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
static int doEponSniffer(int argc, char *argv[], void *p);
#endif
static int doEponDebuglevel(int argc, char *argv[], void *p);
static int doEponDeregister(int argc, char *argv[], void *p);

static int doEponSetTmdrftflag(int argc, char *argv[], void *p);
static int doEponDumpAllReg(int argc, char *argv[], void *p);

#ifdef EPON_MAC_HW_TEST
static int doEponRegtest(int argc, char *argv[], void *p);
static int doEponHwtest(int argc, char *argv[], void *p);
static int doEponRegDefCheck(int argc, char *argv[], void *p);
static int doEponRegNotRstTest(int argc, char *argv[], void *p);
static int doEponLogRstNonCfgReg(int argc, char *argv[], void *p);
static int doEponLogRstCfgReg(int argc, char *argv[], void *p);
static int doEponMacRstTest(int argc, char *argv[], void *p);
static int doEponRegRstTest(int argc, char *argv[], void *p);
static int doEponSetProbe(int argc, char *argv[], void *p);
static int doEponResetMac(int argc, char *argv[], void *p);
static int doEponResetWan(int argc, char *argv[], void *p);
static int doEponSetRgstAckType(int argc, char *argv[], void *p);
static int doEponSetIgnoreIntMask(int argc, char *argv[], void *p);
static int doEponSetDefLlidMask(int argc, char *argv[], void *p);
#endif

#endif //_EPON_CMD_H_