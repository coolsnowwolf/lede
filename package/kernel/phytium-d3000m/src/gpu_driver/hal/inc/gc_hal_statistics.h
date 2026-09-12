/****************************************************************************
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Phytium Corporation. This is proprietary information owned by
*    Phytium Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Phytium Corporation.
*
*****************************************************************************/


#ifndef __gc_hal_statistics_h_
#define __gc_hal_statistics_h_

#define VIV_STAT_ENABLE_STATISTICS              0

/*  Toal number of frames for which the frame time is accounted. We have storage
 *  to keep frame times for last this many frames.
 */
#define VIV_STAT_FRAME_BUFFER_SIZE              30

/*
 *   Total number of frames sampled for a mode. This means
 *
 *   # of frames for HZ Current  : VIV_STAT_EARLY_Z_SAMPLE_FRAMES
 *   # of frames for HZ Switched : VIV_STAT_EARLY_Z_SAMPLE_FRAMES
 * +
 * --------------------------------------------------------
 *                               : (2 * VIV_STAT_EARLY_Z_SAMPLE_FRAMES) frames needed
 *
 *   IMPORTANT: This total must be smaller than VIV_STAT_FRAME_BUFFER_SIZE
 */
#define VIV_STAT_EARLY_Z_SAMPLE_FRAMES          7
#define VIV_STAT_EARLY_Z_LATENCY_FRAMES         2

/*
 * Multiplication factor for previous Hz off mode.
 * Make it more than 1.0 to advertise HZ on.
 */
#define VIV_STAT_EARLY_Z_FACTOR                 (1.05f)

/* HAL statistics information. */
typedef struct _gcsSTATISTICS_EARLYZ {
    gctUINT                     switchBackCount;
    gctUINT                     nextCheckPoint;
    gctBOOL                     disabled;
} gcsSTATISTICS_EARLYZ;

/* HAL statistics information. */
typedef struct _gcsSTATISTICS {
    gctUINT64                   frameTime[VIV_STAT_FRAME_BUFFER_SIZE];
    gctUINT64                   previousFrameTime;
    gctUINT                     frame;
    gcsSTATISTICS_EARLYZ        earlyZ;
} gcsSTATISTICS;

/* Add a frame based data into current statistics. */
void
gcfSTATISTICS_AddData(IN gceSTATISTICS Key, IN gctUINT Value);

/* Marks the frame end and triggers statistical calculations and decisions.*/
void
gcfSTATISTICS_MarkFrameEnd(void);

/* Sets whether the dynamic HZ is disabled or not .*/
void
gcfSTATISTICS_DisableDynamicEarlyZ(IN gctBOOL Disabled);

#endif /*__gc_hal_statistics_h_ */
