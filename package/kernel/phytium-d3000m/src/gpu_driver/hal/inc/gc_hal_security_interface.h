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


#ifndef _GC_HAL_SECURITY_INTERFACE_H_
#define _GC_HAL_SECURITY_INTERFACE_H_
struct kernel_start_command {
    kernel_packet_command_t command; /*! The command (always needs to be the first entry in a structure). */
    gctUINT8                gpu;     /*! Which GPU. */
    gctADDRESS              address;
    gctUINT32               bytes;
};

/*!
 *  @brief gckCOMMAND Object requests TrustZone to submit command buffer.
 *  @discussion
 *  Code in trustzone will check content of command buffer after copying command buffer to TrustZone.
 */
struct kernel_submit {
    kernel_packet_command_t command;                /*! The command (always needs to be the first entry in a structure). */
    gctUINT8                gpu;                    /*! Which GPU. */
    gctUINT8                kernel_command;         /*! Whether it is a kernel command. */
    gctUINT32               command_buffer_handle;  /*! Handle to command buffer. */
    gctUINT32               offset;                 /* Offset in command buffer. */
    gctUINT32               *command_buffer;        /*! Content of command buffer need to be submit. */
    gctUINT32               command_buffer_length;  /*! Length of command buffer. */
};

/*!
 *  @brief gckVIDMEM Object requests TrustZone to allocate security memory.
 *  @discussion
 *  Allocate a buffer from security GPU memory.
 */
struct kernel_allocate_security_memory {
    kernel_packet_command_t command;        /*! The command (always needs to be the first entry in a structure). */
    gctUINT32               bytes;          /*! Requested bytes. */
    gctUINT32               memory_handle;  /*! Handle of allocated memory. */
};

/*!
 *  @brief gckVIDMEM Object requests TrustZone to allocate security memory.
 *  @discussion
 *  Free a video memory buffer from security GPU memory.
 */
struct kernel_free_security_memory {
    kernel_packet_command_t command;        /*! The command (always needs to be the first entry in a structure). */
    gctUINT32               memory_handle;  /*! Handle of allocated memory. */
};

struct kernel_execute {
    kernel_packet_command_t command;                /*! The command (always needs to be the first entry in a structure). */
    gctUINT8                gpu;                    /*! Which GPU. */
    gctUINT8                kernel_command;         /*! Whether it is a kernel command. */
    gctUINT32               *command_buffer;        /*! Content of command buffer need to be submit. */
    gctUINT32               command_buffer_length;  /*! Length of command buffer. */
};

typedef struct kernel_map_scatter_gather {
    gctUINT32                         bytes;
    gctUINT32                         physical;
    struct kernel_map_scatter_gather  *next;
} kernel_map_scatter_gather_t;

struct kernel_map_memory {
    kernel_packet_command_t      command;
    kernel_map_scatter_gather_t  *scatter;
    gctUINT32                    *physicals;
    gctPHYS_ADDR_T               physical;  /*! Contiguous physical address range. */
    gctUINT32                    pageCount;
    gctADDRESS                   gpuAddress;
};

struct kernel_unmap_memory {
    gctADDRESS gpuAddress;
    gctUINT32  pageCount;
};

struct kernel_read_mmu_exception {
    gctUINT32 mmuStatus;
    gctUINT32 mmuException;
};

struct kernel_handle_mmu_exception {
    gctUINT32      mmuStatus;
    gctPHYS_ADDR_T physical;
    gctADDRESS     gpuAddress;
};

typedef struct _gcsTA_INTERFACE {
    kernel_packet_command_t command;
    union {
        struct kernel_submit                   Submit;
        struct kernel_start_command            StartCommand;
        struct kernel_allocate_security_memory AllocateSecurityMemory;
        struct kernel_execute                  Execute;
        struct kernel_map_memory               MapMemory;
        struct kernel_unmap_memory             UnmapMemory;
        struct kernel_read_mmu_exception       ReadMMUException;
        struct kernel_handle_mmu_exception     HandleMMUException;
    } u;
    gceSTATUS result;
} gcsTA_INTERFACE;

#endif
