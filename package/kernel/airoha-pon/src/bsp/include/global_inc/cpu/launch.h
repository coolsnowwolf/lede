
/************************************************************************
 *
 *  loader_api.h
 *
 *  API file for load image module
 *
 * ######################################################################
 *
 * mips_start_of_header
 * 
 *  $Id: launch.h,v 1.6 2008-06-26 22:16:44 chris Exp $
 * 
 * Copyright (c) [Year(s)] MIPS Technologies, Inc. All rights reserved.
 *
 * Unpublished rights reserved under U.S. copyright law.
 *
 * PROPRIETARY/SECRET CONFIDENTIAL INFORMATION OF MIPS TECHNOLOGIES,
 * INC. FOR INTERNAL USE ONLY.
 *
 * Under no circumstances (contract or otherwise) may this information be
 * disclosed to, or copied, modified or used by anyone other than employees
 * or contractors of MIPS Technologies having a need to know.
 *
 * 
 * mips_end_of_header
 *
 ************************************************************************/


#ifndef LAUNCH_H
#define LAUNCH_H

#ifndef __ASSEMBLY__

struct cpulaunch {
    unsigned long	pc;
    unsigned long	gp;
    unsigned long	sp;
    unsigned long	a0;
    unsigned long	_pad[3]; /* pad to cache line size to avoid thrashing */
    unsigned long	flags;
};

#else

#define LOG2CPULAUNCH	5
#define	LAUNCH_PC	0
#define	LAUNCH_GP	4
#define	LAUNCH_SP	8
#define	LAUNCH_A0	12
#define	LAUNCH_FLAGS	28

#endif

#define LAUNCH_FREADY	1
#define LAUNCH_FGO	    2
#define LAUNCH_FGONE	4

#define SCRLAUNCH	0x00000e00
#define CPULAUNCH	0x00000f00
#define NCPULAUNCH	8

/* Polling period in count cycles for secondary CPU's */
#define LAUNCHPERIOD	10000	

/* use GDMP SRAM for CPU 1,2,3's waiting code and launch flags */
#define	GDMP_SRAM_BASE          0xBFA40000
#define BUSY_WAIT_ZONE_START    0x040	
#define BUSY_WAIT_ZONE_SIZE     0x100
#define BUSY_WAIT_ZONE_BASE     (GDMP_SRAM_BASE+BUSY_WAIT_ZONE_START) /*0xbfa40040*/
#define CPU_LAUNCH_BASE         (GDMP_SRAM_BASE+BUSY_WAIT_ZONE_START+BUSY_WAIT_ZONE_SIZE) /*0xbfa40140*/

#endif /* LAUNCH_H */
