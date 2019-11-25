/*
 * netlink/list.h	Netlink List Utilities
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_LIST_H_
#define NETLINK_LIST_H_

struct nl_list_head
{
	struct nl_list_head *	next;
	struct nl_list_head *	prev;
};


static inline void __nl_list_add(struct nl_list_head *obj,
				 struct nl_list_head *prev,
				 struct nl_list_head *next)
{
	prev->next = obj;
	obj->prev = prev;
	next->prev = obj;
	obj->next = next;
}

static inline void nl_list_add_tail(struct nl_list_head *obj,
				    struct nl_list_head *head)
{
	__nl_list_add(obj, head->prev, head);
}

static inline void nl_list_add_head(struct nl_list_head *obj,
				    struct nl_list_head *head)
{
	__nl_list_add(obj, head, head->next);
}

static inline void nl_list_del(struct nl_list_head *obj)
{
	obj->next->prev = obj->prev;
	obj->prev->next = obj->next;
}

static inline int nl_list_empty(struct nl_list_head *head)
{
	return head->next == head;
}

#define nl_container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - ((size_t) &((type *)0)->member));})

#define nl_list_entry(ptr, type, member) \
	nl_container_of(ptr, type, member)

#define nl_list_at_tail(pos, head, member) \
	((pos)->member.next == (head))

#define nl_list_at_head(pos, head, member) \
	((pos)->member.prev == (head))

#define NL_LIST_HEAD(name) \
	struct nl_list_head name = { &(name), &(name) }

#define nl_list_first_entry(head, type, member)			\
	nl_list_entry((head)->next, type, member)

#define nl_list_for_each_entry(pos, head, member)				\
	for (pos = nl_list_entry((head)->next, typeof(*pos), member);	\
	     &(pos)->member != (head); 	\
	     (pos) = nl_list_entry((pos)->member.next, typeof(*(pos)), member))

#define nl_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = nl_list_entry((head)->next, typeof(*pos), member),	\
		n = nl_list_entry(pos->member.next, typeof(*pos), member);	\
	     &(pos)->member != (head); 					\
	     pos = n, n = nl_list_entry(n->member.next, typeof(*n), member))

#define nl_init_list_head(head) \
	do { (head)->next = (head); (head)->prev = (head); } while (0)

#endif
