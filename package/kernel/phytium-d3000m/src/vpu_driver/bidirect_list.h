/* SPDX-License-Identifier: GPL-2.0 */
/*------------------------------------------------------------------------------
 *--                                                                          --
 *--       This software is confidential and proprietary and may be used      --
 *--        only as expressly authorized by a licensing agreement from        --
 *--                                                                          --
 *--            Copyright (C) 2024-2025, Phytium Technology Co., Ltd.         --
 *--                                                                          --
 *--                 The entire notice above must be reproduced               --
 *--                  on all copies and should not be removed.                --
 *--                                                                          --
 *------------------------------------------------------------------------------
 *--
 *--  Abstract : bidirection list header (kernel module)
 *--
 *------------------------------------------------------------------------------
 */

#ifndef _BIDIRECT_LIST_H_
#define _BIDIRECT_LIST_H_
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/*
 * Macros to help debugging
 */

#undef PDEBUG /* undef it, just in case */
#ifdef BIDIRECTION_LIST_DEBUG
#ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) pr_info("hmp4e: " fmt, ##args)
#else
/* This one for user space */
#define PDEBUG(fmt, args...) printf(__FILE__ ":%d: " fmt, __LINE__, ##args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

/******************************************************************************\
 * <Typedefs>
\******************************************************************************/
typedef struct bi_list_node {
	void *data;
	struct bi_list_node *next;
	struct bi_list_node *previous;
} bi_list_node;
typedef struct bi_list {
	bi_list_node *head;
	bi_list_node *tail;
} bi_list;

void init_bi_list(bi_list *list);

bi_list_node *bi_list_create_node(void);

void bi_list_free_node(bi_list_node *node);

void bi_list_insert_node_tail(bi_list *list, bi_list_node *current_node);

void bi_list_insert_node_before(bi_list *list, bi_list_node *base_node,
				bi_list_node *new_node);

void bi_list_remove_node(bi_list *list, bi_list_node *current_node);

#endif /* !_BIDIRECT_LIST_H_ */
