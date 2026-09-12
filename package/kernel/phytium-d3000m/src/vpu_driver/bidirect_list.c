// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 vcmd driver.
 *
 *    Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License, version 2, as
 *    published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License version 2 for more details.
 *
 *    You may obtain a copy of the GNU General Public License
 *    Version 2 at the following locations:
 *    https://opensource.org/licenses/gpl-2.0.php
 */
#include <linux/kernel.h>
#include <linux/module.h>
/* needed for __init,__exit directives */
#include <linux/init.h>
/* needed for remap_page_range
 *SetPageReserved
 *ClearPageReserved
 */
#include <linux/mm.h>
/* obviously, for kmalloc */
#include <linux/slab.h>
/* for struct file_operations, register_chrdev() */
#include <linux/fs.h>
/* standard error codes */
#include <linux/errno.h>

#include <linux/moduleparam.h>
/* request_irq(), free_irq() */
#include <linux/interrupt.h>
#include <linux/sched.h>

#include <linux/semaphore.h>
#include <linux/spinlock.h>
/* needed for virt_to_phys() */
#include <asm/io.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>

#include <asm/irq.h>

#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/timer.h>

#include "bidirect_list.h"

void init_bi_list(bi_list *list)
{
	list->head = NULL;
	list->tail = NULL;
}

bi_list_node *bi_list_create_node(void)
{
	bi_list_node *node = NULL;

	node = vmalloc(sizeof(bi_list_node));
	if (!node) {
		PDEBUG("%s\n", "vmalloc for node fail!");
		return node;
	}
	memset(node, 0, sizeof(bi_list_node));
	return node;
}

void bi_list_free_node(bi_list_node *node)
{
	//free current node
	vfree(node);
}

void bi_list_insert_node_tail(bi_list *list, bi_list_node *current_node)
{
	if (!current_node) {
		PDEBUG("%s\n", "insert node tail  NULL");
		return;
	}
	if (list->tail) {
		current_node->previous = list->tail;
		list->tail->next = current_node;
		list->tail = current_node;
		list->tail->next = NULL;
	} else {
		list->head = current_node;
		list->tail = current_node;
		current_node->next = NULL;
		current_node->previous = NULL;
	}
}

void bi_list_insert_node_before(bi_list *list, bi_list_node *base_node,
				bi_list_node *new_node)
{
	bi_list_node *temp_node_previous = NULL;

	if (!new_node) {
		PDEBUG("%s\n", "insert node before new node NULL");
		return;
	}
	if (base_node) {
		if (base_node->previous) {
			//at middle position
			temp_node_previous = base_node->previous;
			temp_node_previous->next = new_node;
			new_node->next = base_node;
			base_node->previous = new_node;
			new_node->previous = temp_node_previous;
		} else {
			//at head
			base_node->previous = new_node;
			new_node->next = base_node;
			list->head = new_node;
			new_node->previous = NULL;
		}
	} else {
		//at tail
		bi_list_insert_node_tail(list, new_node);
	}
}

void bi_list_remove_node(bi_list *list, bi_list_node *current_node)
{
	bi_list_node *temp_node_previous = NULL;
	bi_list_node *temp_node_next = NULL;

	if (!current_node) {
		PDEBUG("%s\n", "remove node NULL");
		return;
	}
	temp_node_next = current_node->next;
	temp_node_previous = current_node->previous;

	if (!temp_node_next && !temp_node_previous) {
		//there is only one node.
		list->head = NULL;
		list->tail = NULL;
	} else if (!temp_node_next) {
		//at tail
		list->tail = temp_node_previous;
		temp_node_previous->next = NULL;
	} else if (!temp_node_previous) {
		//at head
		list->head = temp_node_next;
		temp_node_next->previous = NULL;
	} else {
		//at middle position
		temp_node_previous->next = temp_node_next;
		temp_node_next->previous = temp_node_previous;
	}
}
