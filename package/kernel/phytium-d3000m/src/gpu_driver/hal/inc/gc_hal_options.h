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


#ifndef __gc_hal_options_h_
#define __gc_hal_options_h_

/*
 *   gcdSECURITY
 */
#ifndef gcdSECURITY
#define gcdSECURITY                     0
#endif

/*
 *   gcdPRINT_VERSION
 *
 *       Print HAL version.
 */
#ifndef gcdPRINT_VERSION
#define gcdPRINT_VERSION                0
#endif

/*
 *   USE_KERNEL_VIRTUAL_BUFFERS
 *
 *       This define enables the use of VM for gckCommand and fence buffers.
 */
#ifndef USE_KERNEL_VIRTUAL_BUFFERS
#if defined(UNDER_CE)
#  define USE_KERNEL_VIRTUAL_BUFFERS  1
# else
#  define USE_KERNEL_VIRTUAL_BUFFERS  1
# endif
#endif

/*
 *   USE_NEW_LINUX_SIGNAL
 *
 *       This define enables the Linux kernel signaling between kernel and user.
 */
#ifndef USE_NEW_LINUX_SIGNAL
#define USE_NEW_LINUX_SIGNAL            0
#endif

/*
 *   USE_LINUX_PCIE
 *
 *       This define enables ftg340 as a Linux PCIE driver.
 */
#ifndef USE_LINUX_PCIE
#define USE_LINUX_PCIE                  0
#endif

/*
 *   PHYTIUM_PROFILER
 *
 *       This define enables the profiler for hardware counters.
 */
#ifndef PHYTIUM_PROFILER
#define PHYTIUM_PROFILER                1
#endif

/*
 *   gcdUSE_VX
 *
 *       Enable VX HAL layer.
 */
#ifndef gcdUSE_VX
#define gcdUSE_VX                       1
#endif

/*
 *   gcdDUMP
 *
 *       Dump for hw capture.
 *       When set to 1, a dump of all states and memory uploads, as well as other
 *       hardware related execution will be printed to the debug console.  This
 *       data can be used for playing back applications.
 *
 *       When set to 2, for vxc, all output memory will be dump.
 *
 *       Please get tweak settings in gc_hal_dump.h.
 */
#ifndef gcdDUMP
#define gcdDUMP                         0
#endif

/*
*   gcdDUMP_DX
* 
*       Enable/Disable DX driver dump for hw capture.
*       To enable the hw capture for DX,
*       the gcdDUMP_DX and gcdDUMP need to be set to 1.
*/
#ifndef gcdDUMP_DX
#define gcdDUMP_DX                      0
#endif

/*
 *   gcdDUMP_IN_KERNEL
 *
 *       Enhanced feature for hw capture.
 *       Required for MCFE.
 *       When set to 1, all dumps will happen in the kernel.  This is handy if
 *       you want the kernel to dump its command buffers as well and the data
 *       needs to be in sync.
 *
 *       Dump in kernel implies kernel command dump.
 *       See debugfs:/gc/dump/ for runtime configuration.
 */
#ifndef gcdDUMP_IN_KERNEL
#define gcdDUMP_IN_KERNEL               0
#endif

/*
 *   gcdDUMP_HW_SUBCOMMAND
 *
 *       Dump for hw command buffer
 *       When set to 1, will dump hw command buffer when GPU/VIP hang.
 */
#ifndef gcdDUMP_HW_SUBCOMMAND
#define gcdDUMP_HW_SUBCOMMAND           0
#endif

/*
 *   gcdDUMP_API
 *
 *       Dump driver level API.
 *       When set to 1, a high level dump of the EGL and GL/VG APs's are
 *       captured.
 *
 *       Please get tweak settings in gc_hal_dump.h.
 */
#ifndef gcdDUMP_API
#define gcdDUMP_API                     0
#endif

/*
 *   gcdDUMP_PER_OPERATION
 *
 *       Operation based dump.
 *
 *       Dump the block as below.
 *       1. Multiple operations belong to the same SW tiling block.
 *       2. Single operation which is NOT in any SW tiling block.
 */
#ifndef gcdDUMP_PER_OPERATION
#define gcdDUMP_PER_OPERATION           0
#endif

/*
 *   gcdDEBUG_OPTION
 *       When set to 1, the debug options are enabled. We must set other MACRO to enable
 *       sub case.
 */
#ifndef gcdDEBUG_OPTION
#define gcdDEBUG_OPTION                 0

#if gcdDEBUG_OPTION
/*
 *   gcdDEBUG_OPTION_KEY
 *       The process name of debug application.
 */
#ifndef gcdDEBUG_OPTION_KEY
#   define gcdDEBUG_OPTION_KEY                 "process"
#  endif
/*
 *   gcdDEBUG_OPTION_NO_GL_DRAWS
 *       When set to 1, all glDrawArrays and glDrawElements will be skip.
 */
#ifndef gcdDEBUG_OPTION_NO_GL_DRAWS
#   define gcdDEBUG_OPTION_NO_GL_DRAWS         0
#  endif
/*
 *   gcdDEBUG_OPTION_NO_DRAW_PRIMITIVES
 *       When set to 1, all DrawPrimitives will be skip.
 */
#ifndef gcdDEBUG_OPTION_NO_DRAW_PRIMITIVES
#   define gcdDEBUG_OPTION_NO_DRAW_PRIMITIVES  0
#  endif
/*
 *   gcdDEBUG_OPTION_SKIP_SWAP
 *       When set to 1, just one out of gcdDEBUG_OPTION_SKIP_FRAMES(such as 1/10)
 *       eglSwapBuffers will be resolve, others skip.
 */
#ifndef gcdDEBUG_OPTION_SKIP_SWAP
#   define gcdDEBUG_OPTION_SKIP_SWAP                   0
#   define gcdDEBUG_OPTION_SKIP_FRAMES                 10
#  endif
/*
 *   gcdDEBUG_OPTION_FORCE_16BIT_RENDER_TARGET
 *       When set to 1, the format of render target will force to RGB565.
 */
#ifndef gcdDEBUG_OPTION_FORCE_16BIT_RENDER_TARGET
#   define gcdDEBUG_OPTION_FORCE_16BIT_RENDER_TARGET   0
#  endif
/*
 *   gcdDEBUG_OPTION_NONE_TEXTURE
 *       When set to 1, the type of texture will be set to
 *       AQ_TEXTURE_SAMPLE_MODE_TYPE_NONE.
 */
#ifndef gcdDEBUG_OPTION_NONE_TEXTURE
#   define gcdDEBUG_OPTION_NONE_TEXTURE    0
#  endif
/*
 *   gcdDEBUG_OPTION_NONE_DEPTH
 *       When set to 1, the depth format of surface will be set to gcvSURF_UNKNOWN.
 */
#ifndef gcdDEBUG_OPTION_NONE_DEPTH
#   define gcdDEBUG_OPTION_NONE_DEPTH      0
#  endif

/*
 *   gcdDEBUG_FORCE_CONTEXT_UPDATE
 *       When set to 1, context will be updated before every commit.
 */
#ifndef gcdDEBUG_FORCE_CONTEXT_UPDATE
#   define gcdDEBUG_FORCE_CONTEXT_UPDATE   0
#  endif

/*
 *   gcdDEBUG_OPTION_SPECIFY_POOL
 *       When set to 1, pool of each type surface can be specified by
 *       changing poolPerType[] in gcsSURF_NODE_Construct.
 */
#ifndef gcdDEBUG_OPTION_SPECIFY_POOL
#   define gcdDEBUG_OPTION_SPECIFY_POOL    0
#  endif

# endif
#endif

/*
 *   gcdENABLE_FSCALE_VAL_ADJUST
 *       When non-zero, FSCALE_VAL when gcvPOWER_ON can be adjusted externally.
 */
#ifndef gcdENABLE_FSCALE_VAL_ADJUST
#define gcdENABLE_FSCALE_VAL_ADJUST             1
#endif

/*
 *   gcdCAPTURE_ONLY_MODE
 *       When non-zero, driver is built with capture only mode.
 *       1) Set DDR address range in capture file with contiguousBase and contiguoutsSize.
 *          Video memory allocation will go through reserved pool with capture only mode.
 *       2) Set SRAM address range in capture file with sRAMBases, sRAMSizes and
 *          extSRAMBases, extSRAMSizes.
 *          Video memory querion will go through reserved pool with capture only mode.
 *       3) VIV: TODO SRAM video memory allocation.
 */
#ifndef gcdCAPTURE_ONLY_MODE
#define gcdCAPTURE_ONLY_MODE                    0
#endif

/*
 *   gcdNULL_DRIVER
 *
 *   Set to 1 for infinite speed hardware.
 *   Set to 2 for bypassing the HAL.
 */
#ifndef gcdNULL_DRIVER
#define gcdNULL_DRIVER                          0
#endif

/*
 *   gcdENABLE_TIMEOUT_DETECTION
 *
 *       Enable timeout detection.
 */
#ifndef gcdENABLE_TIMEOUT_DETECTION
#define gcdENABLE_TIMEOUT_DETECTION             0
#endif

/*
 *   gcdCMD_BUFFER_SIZE
 *
 *       Number of bytes in a command buffer.
 */
#ifndef gcdCMD_BUFFER_SIZE
#if gcdCAPTURE_ONLY_MODE
#  define gcdCMD_BUFFER_SIZE                  (4 << 10)
# else
#  define gcdCMD_BUFFER_SIZE                  (128 << 10)
# endif
#endif

/*
 *   gcdCMD_BLT_BUFFER_SIZE
 *
 *       Number of bytes in a command buffer.
 */
#ifndef gcdCMD_BLT_BUFFER_SIZE
#define gcdCMD_BLT_BUFFER_SIZE                  (1 << 10)
#endif

/*
 *   gcdCMD_BUFFERS
 *
 *       Number of command buffers to use per client.
 */
#ifndef gcdCMD_BUFFERS
#if gcdCAPTURE_ONLY_MODE
#  define gcdCMD_BUFFERS                      1
# else
#  define gcdCMD_BUFFERS                      2
# endif
#endif

/*
 *   gcdMAX_CMD_BUFFERS
 *
 *       Maximum number of command buffers to use per client.
 */
#ifndef gcdMAX_CMD_BUFFERS
#define gcdMAX_CMD_BUFFERS                      8
#endif

/*
 *   gcdCOMMAND_QUEUES
 *
 *       Number of command queues in the kernel.
 */
#ifndef gcdCOMMAND_QUEUES
#define gcdCOMMAND_QUEUES                       2
#endif

/*
 *   gcdPOWER_CONTROL_DELAY
 *
 *       The delay in milliseconds required to wait until the GPU has woke up
 *       from a suspend or power-down state.  This is system dependent because
 *       the bus clock also needs to stabalize.
 */
#ifndef gcdPOWER_CONTROL_DELAY
#define gcdPOWER_CONTROL_DELAY                  0
#endif

/*
 *   gcdREGISTER_READ_FROM_USER
 *   gcdREGISTER_WRITE_FROM_USER
 *
 *       Set to 1 to allow IOCTL calls to get through from user land.  This
 *       should only be in debug or development drops.
 */
#ifndef gcdREGISTER_READ_FROM_USER
#define gcdREGISTER_READ_FROM_USER              1
#endif

#ifndef gcdREGISTER_WRITE_FROM_USER
#define gcdREGISTER_WRITE_FROM_USER             0
#endif

/*
 *   gcdHEAP_SIZE
 *
 *       Set the allocation size for the internal heaps.  Each time a heap is
 *       full, a new heap will be allocated with this minmimum amount of bytes.
 *       The bigger this size, the fewer heaps there are to allocate, the better
 *       the performance.  However, heaps won't be freed until they are
 *       completely free, so there might be some more memory waste if the size is
 *       too big.
 */
#ifndef gcdHEAP_SIZE
#define gcdHEAP_SIZE (64 << 10)
#endif

/*
 *   gcdPOWER_SUSPEND_WHEN_IDLE
 *
 *       Set to 1 to make GPU enter gcvPOWER_SUSPEND when idle detected,
 *       otherwise GPU will enter gcvPOWER_IDLE.
 */
#ifndef gcdPOWER_SUSPEND_WHEN_IDLE
#define gcdPOWER_SUSPEND_WHEN_IDLE              1
#endif

#ifndef gcdFPGA_BUILD
#define gcdFPGA_BUILD                           0
#endif

/*
 *   gcdGPU_TIMEOUT
 *
 *       This define specified the number of milliseconds the system will wait
 *       before it broadcasts the GPU is stuck.  In other words, it will define
 *       the timeout of any operation that needs to wait for the GPU.
 *
 *       If the value is 0, no timeout will be checked for.
 */
#ifndef gcdGPU_TIMEOUT
#if gcdFPGA_BUILD
#  define gcdGPU_TIMEOUT                      2000000
# else
#ifdef EMULATOR
#   define gcdGPU_TIMEOUT                     200000
#  else
#   define gcdGPU_TIMEOUT                     20000
#  endif
# endif
#endif

/*
 *   gcdGPU_ADVANCETIMER
 *
 *       it is advance timer.
 */
#ifndef gcdGPU_ADVANCETIMER
#define gcdGPU_ADVANCETIMER                     250
#endif

/*
 *   gcdSTATIC_LINK
 *
 *       This define disalbes static linking;
 */
#ifndef gcdSTATIC_LINK
#define gcdSTATIC_LINK                          0
#endif

/*
 *   gcdUSE_NEW_HEAP
 *
 *       Setting this define to 1 enables new heap.
 */
#ifndef gcdUSE_NEW_HEAP
#define gcdUSE_NEW_HEAP                         0
#endif

/*
 *   gcdENABLE_BUFFER_ALIGNMENT
 *
 *   When enabled, video memory is allocated with at least 16KB alignment
 *   between multiple sub-buffers.
 */
#ifndef gcdENABLE_BUFFER_ALIGNMENT
#if gcdCAPTURE_ONLY_MODE
#  define gcdENABLE_BUFFER_ALIGNMENT          0
# else
#  define gcdENABLE_BUFFER_ALIGNMENT          1
# endif
#endif

/*
 *   gcdENABLE_BANK_ALIGNMENT
 *
 *   When enabled, video memory is allocated bank aligned. The vendor can modify
 *   _GetSurfaceBankAlignment() and _GetBankOffsetBytes() to define how
 *   different types of allocations are bank and channel aligned.
 *   When disabled (default), no bank alignment is done.
 *   For CAPTURE ONLY MODE, should make sure that gcdENABLE_BANK_ALIGNMENT is disabled.
 */
#ifndef gcdENABLE_BANK_ALIGNMENT
#if gcdCAPTURE_ONLY_MODE
#  define gcdENABLE_BANK_ALIGNMENT            0
# else
#  define gcdENABLE_BANK_ALIGNMENT            0
# endif
#endif

/*
 *   gcdBANK_BIT_START
 *
 *   Specifies the start bit of the bank (inclusive).
 */
#ifndef gcdBANK_BIT_START
#define gcdBANK_BIT_START                       12
#endif

/*
 *   gcdBANK_BIT_END
 *
 *   Specifies the end bit of the bank (inclusive).
 */
#ifndef gcdBANK_BIT_END
#define gcdBANK_BIT_END                         14
#endif

/*
 *   gcdBANK_CHANNEL_BIT
 *
 *   When set, video memory when allocated bank aligned is allocated such that
 *   render and depth buffer addresses alternate on the channel bit specified.
 *   This option has an effect only when gcdENABLE_BANK_ALIGNMENT is enabled.
 *   When disabled (default), no alteration is done.
 */
#ifndef gcdBANK_CHANNEL_BIT
#define gcdBANK_CHANNEL_BIT                     7
#endif

/*
 *   gcdDYNAMIC_SPEED
 *
 *       When non-zero, it informs the kernel driver to use the speed throttling
 *       broadcasting functions to inform the system the GPU should be spet up or
 *       slowed down. It will send a broadcast for slowdown each "interval"
 *       specified by this define in milliseconds
 *       (gckOS_BroadcastCalibrateSpeed).
 */
#ifndef gcdDYNAMIC_SPEED
#define gcdDYNAMIC_SPEED                        2000
#endif

/*
 *   gcdDYNAMIC_EVENT_THRESHOLD
 *
 *       When non-zero, it specifies the maximum number of available events at
 *       which the kernel driver will issue a broadcast to speed up the GPU
 *       (gckOS_BroadcastHurry).
 */
#ifndef gcdDYNAMIC_EVENT_THRESHOLD
#define gcdDYNAMIC_EVENT_THRESHOLD              5
#endif

/*
 *   gcdENABLE_PROFILING
 *
 *       Enable profiling macros.
 */
#ifndef gcdENABLE_PROFILING
#define gcdENABLE_PROFILING                     0
#endif

/*
 *   gcdENABLE_128B_MERGE
 *
 *       Enable 128B merge for the BUS control.
 */
#ifndef gcdENABLE_128B_MERGE
#define gcdENABLE_128B_MERGE                    0
#endif

/*
 *   gcdFRAME_DB
 *
 *       When non-zero, it specified the number of frames inside the frame
 *       database. The frame DB will collect per-frame timestamps and hardware
 *       counters.
 */
#ifndef gcdFRAME_DB
#define gcdFRAME_DB                             0
#define gcdFRAME_DB_RESET                       0
#define gcdFRAME_DB_NAME                        "/var/log/frameDB.log"
#endif

/*
 *  gcdENABLE_CACHEABLE_COMMAND_BUFFER
 *
 *       When non-zero, command buffer will be cacheable.
 */
#ifndef gcdENABLE_CACHEABLE_COMMAND_BUFFER
#define gcdENABLE_CACHEABLE_COMMAND_BUFFER      0
#endif

/*
 *  gcdENABLE_BUFFERABLE_VIDEO_MEMORY
 *
 *       When non-zero, all video memory will be bufferable by default.
 */
#ifndef gcdENABLE_BUFFERABLE_VIDEO_MEMORY
#define gcdENABLE_BUFFERABLE_VIDEO_MEMORY       1
#endif

/*
 *   gcdPOWEROFF_TIMEOUT
 *
 *       When non-zero, GPU will power off automatically from
 *       idle state, and gcdPOWEROFF_TIMEOUT is also the default
 *       timeout in milliseconds.
 */
#ifndef gcdPOWEROFF_TIMEOUT
#define gcdPOWEROFF_TIMEOUT                     300
#endif

/*
 *   QNX_SINGLE_THREADED_DEBUGGING
 */
#ifndef QNX_SINGLE_THREADED_DEBUGGING
#define QNX_SINGLE_THREADED_DEBUGGING           0
#endif

#ifndef gcdUSE_PVR
#define gcdUSE_PVR                              1
#endif

/*
 *   gcdSMALL_BLOCK_SIZE
 *
 *       When non-zero, a part of VIDMEM will be reserved for requests
 *       whose requesting size is less than gcdSMALL_BLOCK_SIZE.
 *
 *       For Linux, it's the size of a page. If this requeset fallbacks
 *       to gcvPOOL_VIRTUAL, memory will be wasted
 *       because they allocate a page at least.
 */
#ifndef gcdSMALL_BLOCK_SIZE
#define gcdSMALL_BLOCK_SIZE                     4096
#define gcdRATIO_FOR_SMALL_MEMORY               32
#endif

/*
 *   gcdENABLE_VIRTUAL_ADDR_UNMAP
 *       enable virtual address unmap for the weight_bias and the virtual image
 */
#ifndef gcdENABLE_VIRTUAL_ADDRESS_UNMAP
#define gcdENABLE_VIRTUAL_ADDRESS_UNMAP         0
#endif

/*
 *   gcdCONTIGUOUS_SIZE_LIMIT
 *       When non-zero, size of video node from gcvPOOL_VIRTUAL contiguous is
 *       limited by gcdCONTIGUOUS_SIZE_LIMIT.
 */
#ifndef gcdCONTIGUOUS_SIZE_LIMIT
#define gcdCONTIGUOUS_SIZE_LIMIT                0
#endif

/*
 *   gcdLINK_QUEUE_SIZE
 *
 *       When non-zero, driver maintains a queue to record information of
 *       latest lined context buffer and command buffer. Data in this queue
 *       is be used to debug.
 */
#ifndef gcdLINK_QUEUE_SIZE
#define gcdLINK_QUEUE_SIZE                      64
#endif

/*  gcdALPHA_KILL_IN_SHADER
 *
 *       Enable alpha kill inside the shader. This will be set automatically by the
 *       HAL if certain states match a criteria.
 */
#ifndef gcdALPHA_KILL_IN_SHADER
#define gcdALPHA_KILL_IN_SHADER                 1
#endif

#ifndef gcdPRINT_SWAP_TIME
#define gcdPRINT_SWAP_TIME                      0
#endif

/*
 *   gcdDVFS
 *
 *       When non-zero, software will make use of dynamic voltage and
 *       frequency feature.
 */
#ifndef gcdDVFS
#define gcdDVFS                                 0
#define gcdDVFS_ANAYLSE_WINDOW                  4
#define gcdDVFS_POLLING_TIME                    (gcdDVFS_ANAYLSE_WINDOW * 4)
#endif

#ifndef gcdSYNC
#define gcdSYNC                                 1
#endif

#ifndef gcdSHADER_SRC_BY_MACHINECODE
#define gcdSHADER_SRC_BY_MACHINECODE            1
#endif

#ifndef gcdGLB27_SHADER_REPLACE_OPTIMIZATION
#define gcdGLB27_SHADER_REPLACE_OPTIMIZATION    1
#endif

/*
 *   gcdGPU_LINEAR_BUFFER_ENABLED
 *
 *       Use linear buffer for GPU apps so HWC can do 2D composition.
 *       Android only.
 */
#ifndef gcdGPU_LINEAR_BUFFER_ENABLED
#define gcdGPU_LINEAR_BUFFER_ENABLED            1
#endif

/*
 *   gcdENABLE_RENDER_INTO_WINDOW
 *
 *       Enable Render-Into-Window (ie, No-Resolve) feature on android.
 *       NOTE that even if enabled, it still depends on hardware feature and
 *       android application behavior. When hardware feature or application
 *       behavior can not support render into window mode, it will fail back
 *       to normal mode.
 *       When Render-Into-Window is finally used, window back buffer of android
 *       applications will be allocated matching render target tiling format.
 *       Otherwise buffer tiling is decided by the above option
 *       'gcdGPU_LINEAR_BUFFER_ENABLED'.
 *       Android only for now.
 */
#ifndef gcdENABLE_RENDER_INTO_WINDOW
#define gcdENABLE_RENDER_INTO_WINDOW            1
#endif

/*
 *   gcdENABLE_RENDER_INTO_WINDOW_WITH_FC
 *
 *       Enable Direct-rendering (ie, No-Resolve) with tile status.
 *       This is expremental and in development stage.
 *       This will dynamically check if color compression is available.
 */
#ifndef gcdENABLE_RENDER_INTO_WINDOW_WITH_FC
#define gcdENABLE_RENDER_INTO_WINDOW_WITH_FC    1
#endif

/*
 *   gcdENABLE_BLIT_BUFFER_PRESERVE
 *
 *       Render-Into-Window (ie, No-Resolve) does not include preserved swap
 *       behavior.  This feature can enable buffer preserve in No-Resolve mode.
 *       When enabled, previous buffer (may be part of ) will be resolve-blitted
 *       to current buffer.
 */
#ifndef gcdENABLE_BLIT_BUFFER_PRESERVE
#define gcdENABLE_BLIT_BUFFER_PRESERVE          1
#endif

/*
 *   gcdANDROID_NATIVE_FENCE_SYNC
 *
 *       Enable android native fence sync. It is introduced since jellybean-4.2.
 *       Depends on linux kernel option: CONFIG_SYNC.
 *
 *       0: Disabled
 *       1: Build framework for native fence sync feature, and EGL extension
 *       2: Enable async swap buffers for client
 *          * Native fence sync for client 'queueBuffer' in EGL, which is
 *            'acquireFenceFd' for layer in compositor side.
 *       3. Enable async hwcomposer composition.
 *          * 'releaseFenceFd' for layer in compositor side, which is native
 *            fence sync when client 'dequeueBuffer'
 *          * Native fence sync for compositor 'queueBuffer' in EGL, which is
 *            'acquireFenceFd' for framebuffer target for DC
 */
#ifndef gcdANDROID_NATIVE_FENCE_SYNC
#define gcdANDROID_NATIVE_FENCE_SYNC            0
#endif

#ifndef gcdLINUX_SYNC_FILE
#define gcdLINUX_SYNC_FILE                      0
#endif

/*
 *   gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
 *
 *       Enable implicit android native buffer sync.
 *
 *       For non-HW_RENDER buffer, CPU (or other hardware) and GPU can access
 *       the buffer at the same time. This is to add implicit synchronization
 *       between CPU (or the hardware) and GPU.
 *
 *       Eventually, please do not use implicit native buffer sync, but use
 *       "fence sync" or "android native fence sync" instead in libgui, which
 *       can be enabled in frameworks/native/libs/gui/Android.mk. This kind
 *       of synchronization should be done by app but not driver itself.
 *
 *       Please disable this option when either "fence sync" or
 *       "android native fence sync" is enabled.
 */
#ifndef gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
#define gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC  1
#endif

/*
 * Implicit native buffer sync is not needed when ANDROID_native_fence_sync
 * is available.
 */
#if gcdANDROID_NATIVE_FENCE_SYNC
#undef gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
#define gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC  0
#endif

/*
 *   gcdUSE_WCLIP_PATCH
 *
 *       Enable wclipping patch.
 */
#ifndef gcdUSE_WCLIP_PATCH
#define gcdUSE_WCLIP_PATCH              1
#endif

#ifndef gcdUSE_NPOT_PATCH
#define gcdUSE_NPOT_PATCH               1
#endif

/*
 *   gcdINTERNAL_COMMENT
 *
 *       Wrap internal comment, content wrapped by it and the macor itself
 *       will be removed in release driver.
 */
#ifndef gcdINTERNAL_COMMENT
#define gcdINTERNAL_COMMENT             1
#endif

/*
 *   gcdRTT_DISABLE_FC
 *
 *       Disable RTT FC support. For test only.
 */
#ifndef gcdRTT_DISABLE_FC
#define gcdRTT_DISABLE_FC               0
#endif

/*
 *   gcdFORCE_MIPMAP
 *
 *       Force generate mipmap for texture.
 */
#ifndef gcdFORCE_MIPMAP
#define gcdFORCE_MIPMAP                 0
#endif

/*
 *   gcdFORCE_BILINEAR
 *
 *       Force bilinear for mipfilter.
 */
#ifndef gcdFORCE_BILINEAR
#define gcdFORCE_BILINEAR               1
#endif

/*
 *   gcdBINARY_TRACE
 *
 *       When non-zero, binary trace will be generated.
 *
 *       When gcdBINARY_TRACE_FILE_SIZE is non-zero, binary trace buffer will
 *       be written to a file which size is limited to
 *       gcdBINARY_TRACE_FILE_SIZE.
 */
#ifndef gcdBINARY_TRACE
#define gcdBINARY_TRACE                 0
#define gcdBINARY_TRACE_FILE_SIZE       0
#endif

/*  gcdINTERRUPT_STATISTIC
 *
 *  Monitor the event send to GPU and interrupt issued by GPU.
 */

#ifndef gcdINTERRUPT_STATISTIC
#if defined(LINUX) || defined(__QNXNTO__) || defined(UNDER_CE) || defined(__VXWORKS__)
#  define gcdINTERRUPT_STATISTIC  1
# else
#  define gcdINTERRUPT_STATISTIC  0
# endif
#endif

/*
 *   gcdFENCE_WAIT_LOOP_COUNT
 *       Wait fence, loop count.
 */
#ifndef gcdFENCE_WAIT_LOOP_COUNT
#define gcdFENCE_WAIT_LOOP_COUNT    10000
#endif

/*
 *   gcdPARTIAL_FAST_CLEAR
 *       When it's not zero, partial fast clear is enabled.
 *       Depends on gcdHAL_3D_DRAWBLIT, if gcdHAL_3D_DRAWBLIT is not enabled,
 *       only available when scissor box is completely aligned.
 *       Expremental, under test only. Not ready for production.
 */
#ifndef gcdPARTIAL_FAST_CLEAR
#define gcdPARTIAL_FAST_CLEAR       0
#endif

/*
 *   gcdTEST_DEC200
 *       Test part for DEC200. Remove when release.
 */
#ifndef gcdTEST_DEC200
#define gcdTEST_DEC200              0
#endif

/*
 *   gcdPATTERN_FAST_PATH
 *        For pattern match
 */
#ifndef gcdPATTERN_FAST_PATH
#define gcdPATTERN_FAST_PATH        1
#endif

/*
 *   gcdPERFORMANCE_ANALYSIS
 *
 *       When set to 1, driver will pass information through loadstate
 *       to HW. This loadstate does not impact HW execution.
 */
#ifndef gcdPERFORMANCE_ANALYSIS
#define gcdPERFORMANCE_ANALYSIS     0
#endif

/*
 *   gcdFRAMEINFO_STATISTIC
 *       When enable, collect frame information.
 */
#ifndef gcdFRAMEINFO_STATISTIC

#if (defined(DBG) && DBG) || defined(DEBUG)               || \
     defined(_DEBUG) || gcdDUMP || gcdPERFORMANCE_ANALYSIS || \
     (defined(WIN32) && !defined(UNDER_CE))                || \
     gcdFPGA_BUILD
#  define gcdFRAMEINFO_STATISTIC  1
# else
#  define gcdFRAMEINFO_STATISTIC  1
# endif

#endif

/*
 *   gcdDEC_ENABLE_AHB
 *       Enable DEC300 compression AHB mode or not.
 */
#ifndef gcdDEC_ENABLE_AHB
#define gcdDEC_ENABLE_AHB           0
#endif

/*
 *   gcdENABLE_UNIFIED_CONSTANT
 *       Enable unified constant or not.
 */
#ifndef gcdENABLE_UNIFIED_CONSTANT
#define gcdENABLE_UNIFIED_CONSTANT  1
#endif

/*
 *   Core configurations. By default enable all cores.
 */
#ifndef gcdENABLE_3D
#define gcdENABLE_3D                1
#endif

/*
 *   gcdRECORD_COMMAND
 */
#ifndef gcdRECORD_COMMAND
#define gcdRECORD_COMMAND                       0
#endif

/*
 *   gcdALLOC_CMD_FROM_RESERVE
 *
 *   Provide a way by which location of command buffer can be
 *   specified. This is a DEBUG option to limit command buffer
 *   to some memory range.
 */
#ifndef gcdALLOC_CMD_FROM_RESERVE
#define gcdALLOC_CMD_FROM_RESERVE               0
#endif

/*
 *   gcdBOUNDARY_CHECK
 *
 *   When enabled, add bounary before and after a range of
 *   GPU address. So overflow can be trapped by MMU exception.
 *   This is a debug option for new MMU and gcdUSE_MMU_EXCEPTION
 *   is enabled.
 */
#ifndef gcdBOUNDARY_CHECK
#define gcdBOUNDARY_CHECK                       0
#endif

/*
 *   gcdRENDER_QUALITY_CHECK
 *
 *   When enabled, we disable performance opt patch
 *   to get know rendering quality comparing with other vendor.
 */
#ifndef gcdRENDER_QUALITY_CHECK
#define gcdRENDER_QUALITY_CHECK                 0
#endif

/*
 *   gcdSYSTRACE
 *
 *   When enabled, we embed systrace in function header/footer
 *   to gather time information on linux platforms include android.
 *   '1' to trace API (EGL, ES11, ES2x, ES3x, etc)
 *   '2' to trace HAL (except compiler)
 *   '4' to trace HAL compiler
 *   See gc_hal_user_debug.c for more detailed trace zones.
 */
#ifndef gcdSYSTRACE
#define gcdSYSTRACE                             0
#endif

#ifndef gcdENABLE_APPCTXT_BLITDRAW
#define gcdENABLE_APPCTXT_BLITDRAW              0
#endif



/*
 *   gcdENABLE_TRUST_APPLICATION
 *
 *   When enabled, trust application is used to handle 'security' registers.
 *
 *   1) If HW doesn't have robust and security feature, this option is meaningless.
 *   2) If HW have robust and security and this option is not enable,
 *      security registers are handled by non secure driver. It is for
 *      platform doesn't want/need to use trust zone.
 */
#ifndef gcdENABLE_TRUST_APPLICATION
#  define gcdENABLE_TRUST_APPLICATION           0
#endif

/* Disable gcdENABLE_TRUST_APPLICATION when oboslete gcdSECURITY enabled. */
#if gcdSECURITY
# undef gcdENABLE_TRUST_APPLICATION
# define gcdENABLE_TRUST_APPLICATION            0
#endif

#ifndef gcdMMU_SECURE_AREA_SIZE
#if defined(gcdENABLE_MMU_1KMODE)
#  define gcdMMU_SECURE_AREA_SIZE             32
# else
#  define gcdMMU_SECURE_AREA_SIZE             128
# endif
#endif

#ifndef gcdUSE_MMU_EXCEPTION
#define gcdUSE_MMU_EXCEPTION                    1
#endif

#ifndef gcdVX_OPTIMIZER
#define gcdVX_OPTIMIZER                         0
#endif

#ifndef gcdALLOC_ON_FAULT
#define gcdALLOC_ON_FAULT                       0
#endif

/*
 *   gcdDISABLE_GPU_VIRTUAL_ADDRESS
 *
 *       When enabled, disable MMU and all virtual allocated from MMU.
 */
#ifndef gcdDISABLE_GPU_VIRTUAL_ADDRESS
#define gcdDISABLE_GPU_VIRTUAL_ADDRESS          0
#endif

/*
 *   gcdENABLE_KERNEL_FENCE
 *       When enabled, use kernel fence to do resource tracking.
 */
#ifndef gcdENABLE_KENREL_FENCE
#define gcdENABLE_KERNEL_FENCE                  0
#endif

/*
 *   gcdUSE_VXC_BINARY
 *       When enabled, will use prebuilt shader binary in VX driver.
 */
#ifndef gcdUSE_VXC_BINARY
#define gcdUSE_VXC_BINARY                       0
#endif

/*
 *   gcdFEATURE_SANITYCHECK
 *       When enabled, will do hardware feature sanity check, each
 *       used hardware feature should be printed out.
 */
#ifndef gcdFEATURE_SANITYCHECK
#define gcdFEATURE_SANITYCHECK                  0
#endif

/*
 *   PHYTIUM_PROFILER_SYSTEM_MEMORY
 *
 *       This define enables the profiling data for system memory allocated by driver
 */
#ifndef PHYTIUM_PROFILER_SYSTEM_MEMORY
#define PHYTIUM_PROFILER_SYSTEM_MEMORY          1
#define VP_MALLOC_OFFSET                        (16)

#endif

/*
 *   gcdUSE_ZWP_SYNCHRONIZATION
 *
 *       When enabled, will use the zwp_linux_surface_synchronization path,
 *       otherwise switch to old wayland path.
 */
#define gcdUSE_ZWP_SYNCHRONIZATION                  1

/*
 *   gcdUSE_SINGLE_CONTEXT
 *       When enabled, will enable single context.
         Only VX driver use now.
 */
#ifndef gcdUSE_SINGLE_CONTEXT
#define gcdUSE_SINGLE_CONTEXT                   0
#endif

/*
 *   gcdIGNORE_DRIVER_VERSIONS_MISMATCH
 *       When enabled, driver will ignore user and kernel driver version mismatch.
 */
#ifndef gcdIGNORE_DRIVER_VERSIONS_MISMATCH
#define gcdIGNORE_DRIVER_VERSIONS_MISMATCH      1
#endif

/*
 *   gcdEXTERNAL_SRAM_USAGE
 *       '0': User driver queries the whole external SRAM and manages the memory.
 *            Or user driver dynamically allocate the external SRAM with pool type
 *            gcvPOOL_EXTERNAL_SRAM.
 *
 *       '1': External SRAM only can be used for the initial command,
 *            but the external SRAM base and size must be set by customer.
 *            And it only can be used if pool type is gcvPOOL_EXTERNAL_SRAM when
 *            allocating video memory.
 *
 *       '2': To be extended.
 */
#ifndef gcdEXTERNAL_SRAM_USAGE
#define gcdEXTERNAL_SRAM_USAGE                  0
#endif

/*
 *   gcdENABLE_SW_PREEMPTION
 *       Enable software preemption if set to 1, disable by default.
 *       Only support Linux OS currently.
 */
#ifndef gcdENABLE_SW_PREEMPTION
#define gcdENABLE_SW_PREEMPTION                 0
#endif

/*
 *   gcdSUPPORT_DEVICE_TREE_SOURCE
 *       To suppor device tree feature if set to 1, disable by default.
 *       Only works on linux OS.
 */
#ifndef gcdSUPPORT_DEVICE_TREE_SOURCE
#define gcdSUPPORT_DEVICE_TREE_SOURCE           0
#endif

/*
 *   gcdENABLE_PER_DEVICE_PM
 *       Enable per device power management if set to 2, all the hardware
 *       cores will be one device. Enable per user device power management
 *       if set to 1, the brother cores of a device depends on user driver.
 *       Disable per device power management if set to 0.
 *       Only support Linux OS currently.
 */
#ifndef gcdENABLE_PER_DEVICE_PM
#define gcdENABLE_PER_DEVICE_PM                 0
#endif

/*
 *   gcdUSE_CAPBUF
 */
#ifndef gcdUSE_CAPBUF
#define gcdUSE_CAPBUF                           1
#endif

/*
 *   gcdENABLE_MP_SWITCH
 *       Enable multi-processor mode dynamic switch, the processor count is
 *       determined by specific conditions.
 *       Only support Linux OS currently.
 */
#ifndef gcdENABLE_MP_SWITCH
#define gcdENABLE_MP_SWITCH                     0
#endif

/*
 *   gcdANON_FILE_FOR_ALLOCATOR
 *       Enable this macro can replace the /dev/zero by anon_inode:
 *       [ftg340] in /proc/<pid>/maps.
 *       Without the macro, run 'cat /proc/<pid>/maps' will print "/dev/zero".
 */
#ifndef gcdANON_FILE_FOR_ALLOCATOR
#define gcdANON_FILE_FOR_ALLOCATOR              0
#endif

/*
 *   gcdWAIT_LINK_FE_MODE
 *       0 means always end at the end of commit user command buffer
 *         and reset FE for each commit.
 *       1 means default wait-link mode.
 */
#ifndef gcdWAIT_LINK_FE_MODE
#define gcdWAIT_LINK_FE_MODE                    1
#endif

/*
 *   gcdENABLE_VIDEO_MEMORY_MIRROR
 *       Enable pcie local memory mirror and dma sync.
 *       Only support Linux OS currently.
 */
#ifndef gcdENABLE_VIDEO_MEMORY_MIRROR
#define gcdENABLE_VIDEO_MEMORY_MIRROR           0
#endif
/*
 *   gcdINIT_VIDEO_MEMORY_MIRROR
 *       Init mirror and pcie local memory to characteristic value for debugging.
 *       Depends on gcdENABLE_VIDEO_MEMORY_MIRROR.
 *       Only support Linux OS currently.
 */
#ifndef gcdINIT_VIDEO_MEMORY_MIRROR
#define gcdINIT_VIDEO_MEMORY_MIRROR             0
#endif

/*
 *   gcdSTATIC_VIDEO_MEMORY_MIRROR
 *       Dynamic allocate/free the mirror buffer if set to 0.
 *       Depends on gcdENABLE_VIDEO_MEMORY_MIRROR.
 *       Only support Linux OS currently.
 */
#ifndef gcdSTATIC_VIDEO_MEMORY_MIRROR
#define gcdSTATIC_VIDEO_MEMORY_MIRROR           1
#endif

/*
 *   gcdENABLE_VIDEO_MEMORY_TRACE
 *       Dynamic trace video memory, if set to 1.
 *       Depends on kernel support.
 *       Only support Linux OS currently.
 */
#ifndef gcdENABLE_VIDEO_MEMORY_TRACE
#define gcdENABLE_VIDEO_MEMORY_TRACE            0
#endif

/*
 *   gcdUSER_COMMAND_IN_EXCLUSIVE
 *       User command buffer can be able in exclusive local memory.
 */
#ifndef gcdUSER_COMMAND_IN_EXCLUSIVE
#define gcdUSER_COMMAND_IN_EXCLUSIVE            0
#endif

/*
 *   gcdVALID_COMMAND_BUFFER
 *       If enabled, will check the validity of the command buffer before commit.
 */
#ifndef gcdVALID_COMMAND_BUFFER
#define gcdVALID_COMMAND_BUFFER                 0
#endif

/*
 *   gcdENABLE_MULTI_DEVICE_MANAGEMENT
 *
 *       Manage cores on multiple hardware devices.
 *
 *       Hardware device is composed by a set of cores which are the
 *       same type, such as 3D,  VIP etc. and these cores have
 *       direct connection to work in combined mode.
 */
#ifndef gcdENABLE_MULTI_DEVICE_MANAGEMENT
#define gcdENABLE_MULTI_DEVICE_MANAGEMENT       0
#endif

/*
 *   gcdENABLE_DEVFREQ
 *
 *       Enable linux generic dynamic voltage and frequency scaling.
 *       Now only support frequency scaling.
 */
#ifndef gcdENABLE_DEVFREQ
#define gcdENABLE_DEVFREQ                       0
#endif

/*
 *   gcdSHARED_COMMAND_BUFFER
 *
 *   Share one user command buffer and kernel command buffer for multi-core.
 */
#ifndef gcdSHARED_COMMAND_BUFFER
#define gcdSHARED_COMMAND_BUFFER                 0
#endif

/*
 *   gcdENABLE_AHBXTTD
 *
 *   Enable AHBXTTD register feature. Now only support Xiaomi.
 */
#ifndef gcdENABLE_AHBXTTD
#define gcdENABLE_AHBXTTD                        0
#endif

/*
    gcdENABLE_MEMORY_OPTIMIZATION
        If enabled, will move some object in system memory.
*/
#ifndef gcdENABLE_MEMORY_OPTIMIZATION
#define gcdENABLE_MEMORY_OPTIMIZATION            0
#endif

/*
 *   gcdENABLE_NONCACHE_COMMANDBUF
 *
 *     If enable, map the command buffer without cache 
 */
#ifndef gcdENABLE_NONCACHE_COMMANDBUF
#define gcdENABLE_NONCACHE_COMMANDBUF            0
#endif

/*
 *  gcdENABLE_CLEAR_FENCE
 *
 *      If enabled, will record fence value in kernel and
 *      recovery all fence when the kernel do recovery operation.
 */
#ifndef gcdENABLE_CLEAR_FENCE
#if defined(LINUX) && !defined(EMULATOR)
#  define gcdENABLE_CLEAR_FENCE                  1
# else
#  define gcdENABLE_CLEAR_FENCE                  0
# endif
#endif

#ifndef gcdENABLE_PERF_DISPATCH
#define gcdENABLE_PERF_DISPATCH                  0
#endif

#ifndef gcdHARDWARE_CONTEXT_SWITCH
#define gcdHARDWARE_CONTEXT_SWITCH               0
#endif

/*
 *   gcdENABLE_VM_PASSTHROUGH
 *
 *       When enabled, will use vGPU pass-through(SRIOV) virtualization solution.
 *       Only support Linux OS currently.
 */
#ifndef gcdENABLE_VM_PASSTHROUGH
#define gcdENABLE_VM_PASSTHROUGH                 0
#endif

 /*
  *   gcdENABLE_DYNAMIC_CLUSTERS
  *       Dynamic Switching The Alive Clusters In GPU BLT.
  */
#ifndef gcdENABLE_DYNAMIC_CLUSTERS
#define gcdENABLE_DYNAMIC_CLUSTERS               1
#endif

/*
 *   gcdENABLE_DEFALUT_FC_COMPRESSED
 *       Enable Fast Clear and Compression by defalutfo HW.
 */
#ifndef gcdENABLE_DEFALUT_FC_COMPRESSED
#define gcdENABLE_DEFALUT_FC_COMPRESSED     1
#endif

/*
 *   gcdENABLE_SAMPLER_LOCATION_FIRST_MAPPING
 *       If enabled, sampler uniform location will do map first.
 */
#ifndef gcdENABLE_SAMPLER_LOCATION_FIRST_MAPPING
#define gcdENABLE_SAMPLER_LOCATION_FIRST_MAPPING    1
#endif

#ifndef gcdSUPPORT_DEVICE_ACPI
#define gcdSUPPORT_DEVICE_ACPI 1
#endif

#ifndef gcdENABLE_PHYTIUM_DEVFREQ
#define gcdENABLE_PHYTIUM_DEVFREQ      1
#endif

/*
 *   gcdWINDOWS_GL
 *       When set to 1, debug with Windows GL mode.
 */
#ifndef gcdWINDOWS_GL
#define gcdWINDOWS_GL     0
#endif

#endif /* __gc_hal_options_h_ */
