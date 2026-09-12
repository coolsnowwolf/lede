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


#ifndef __gc_hal_dump_h_
#define __gc_hal_dump_h_

/*
 *  gcdDUMP_KEY
 *
 *      Set this to a string that appears in 'cat /proc/<pid>/cmdline'. E.g. 'camera'.
 *      HAL will create dumps for the processes matching this key.
 */
#ifndef gcdDUMP_KEY
#define gcdDUMP_KEY             "process"
#endif

/*
 *  gcdDUMP_PATH
 *
 *      The dump file location. Some processes cannot write to the sdcard.
 *      Try apps' data dir, e.g. /data/data/com.android.launcher
 */
#ifndef gcdDUMP_PATH
#if defined(ANDROID)
#  define gcdDUMP_PATH        "/mnt/sdcard/"
# else
#  define gcdDUMP_PATH        "/var/tmp/"
# endif
#endif

/*
 *  gcdDUMP_FILE_IN_KERNEL
 *
 *      Default dump file for gcdDUMP_IN_KERNEL.
 *      The file will be writen globally in kernel side.
 *      Can be overwritten in runtime by debugfs:/gc/dump/dump_file
 *
 *      2 pseudo files:
 *      [dmesg]:    means dump to kernel debug message.
 *      [ignored]:  means dump ignored, nothing will be dumpped.
 */
#ifndef gcdDUMP_FILE_IN_KERNEL
#define gcdDUMP_FILE_IN_KERNEL  "[dmesg]"
#endif

/*
 *  gcdDUMP_VERIFY_PER_DRAW
 *
 *      Sub feature of gcdDUMP.
 *      When set to 1, verify RT and images(if used) for every single draw
 *      to ease simulation debug.
 *      Only valid for ES3 driver for now.
 */
#ifndef gcdDUMP_VERIFY_PER_DRAW
#define gcdDUMP_VERIFY_PER_DRAW     0
#endif

 /*
  *  gcdDUMP_VERIFY_PER_FRAME
  *
  *      Sub feature of gcdDUMP.
  *      When set to 1, verify every frame
  *      to ease simulation debug.
  *      Only valid for ES3 driver for now.
  */
#ifndef gcdDUMP_VERIFY_PER_FRAME
#define gcdDUMP_VERIFY_PER_FRAME     0
#endif

/* Standalone dump features below. */

/*
 *  gcdDUMP_FRAMERATE
 *      When set to a value other than zero, averaqe frame rate will be dumped.
 *      The value set is the starting frame that the average will be calculated.
 *      This is needed because sometimes first few frames are too slow to be included
 *      in the average. Frame count starts from 1.
 */
#ifndef gcdDUMP_FRAMERATE
#define gcdDUMP_FRAMERATE           0
#endif

/*
 *  gcdDUMP_FRAME_TGA
 *
 *      When set to a value other than 0, a dump of the frame specified by the value,
 *      will be done into frame.tga. Frame count starts from 1.
 */
#ifndef gcdDUMP_FRAME_TGA
#define gcdDUMP_FRAME_TGA           0
#endif

/*
 *  gcdDUMP_AHB_ACCESS
 *
 *      When set to 1, a dump of all AHB register access will be printed to kernel
 *      message.
 */
#ifndef gcdDUMP_AHB_ACCESS
#define gcdDUMP_AHB_ACCESS          0
#endif

#endif /* __gc_hal_dump_h_ */

