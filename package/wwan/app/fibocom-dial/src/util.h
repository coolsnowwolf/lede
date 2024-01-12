/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stddef.h>

struct listnode {
    struct listnode *next;
    struct listnode *prev;
};

#define node_to_item(node, container, member) \
    (container *)(((char *)(node)) - offsetof(container, member))

#define list_declare(name)   \
    struct listnode name = { \
	.next = &name,       \
	.prev = &name,       \
    }

#define list_for_each(node, list) \
    for (node = (list)->next; node != (list); node = node->next)

#define list_for_each_reverse(node, list) \
    for (node = (list)->prev; node != (list); node = node->prev)
void dbg_time(const char *fmt, ...);
void list_init(struct listnode *list);
void list_add_tail(struct listnode *list, struct listnode *item);
void list_add_head(struct listnode *head, struct listnode *item);
void list_remove(struct listnode *item);

#define list_empty(list) ((list) == (list)->next)
#define list_head(list) ((list)->next)
#define list_tail(list) ((list)->prev)

int epoll_register(int epoll_fd, int fd, unsigned int events);
int epoll_deregister(int epoll_fd, int fd);
#endif
