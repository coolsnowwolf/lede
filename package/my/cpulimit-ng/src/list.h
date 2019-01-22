/**
 *
 * cpulimit - a cpu limiter for Linux
 *
 * Copyright (C) 2005-2008, by:  Angelo Marletta <marlonx80@hotmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 **********************************************************************
 *
 * Dynamic list interface
 *
 */

#ifndef __LIST__

#define __LIST__

#ifndef  TRUE
    #define TRUE 1
    #define FALSE 0
#endif

struct list_node {
    //pointer to the content of the node
    void *data;
    //pointer to previous node
    struct list_node *previous;
    //pointer to next node
    struct list_node *next;
};

struct list {
    //first node
    struct list_node *first;
    //last node
    struct list_node *last;
    //size of the search key in bytes
    int keysize;
    //element count
    int count;
};

/*
 * Initialize a list, with a specified key size
 */
void init_list(struct list *l,int keysize);

/*
 * Add a new element at the end of the list
 * return the pointer to the new node
 */
struct list_node *add_elem(struct list *l,void *elem);

/*
 * Delete a node
 */
void delete_node(struct list *l,struct list_node *node);

/*
 * Delete a node from the list, even the content pointed by it
 * Use only when the content is a dynamically allocated pointer
 */
void destroy_node(struct list *l,struct list_node *node);

/*
 * Check whether a list is empty or not
 */
int is_empty_list(struct list *l);

/*
 * Return the element count of the list
 */
int get_list_count(struct list *l);

/*
 * Return the first element (content of the node) from the list
 */
void *first_elem(struct list *l);

/*
 * Return the first node from the list
 */
struct list_node *first_node(struct list *l);

/*
 * Return the last element (content of the node) from the list
 */
void *last_elem(struct list *l);

/*
 * Return the last node from the list
 */
struct list_node *last_node(struct list *l);

/*
 * Search an element of the list by content
 * the comparison is done from the specified offset and for a specified length
 * if offset=0, the comparison starts from the address pointed by data
 * if length=0, default keysize is used for length
 * if the element is found, return the node address
 * else return NULL
 */
struct list_node *xlocate_node(struct list *l,void *elem,int offset,int length);

/*
 * The same of xlocate_node(), but return the content of the node
 */
void *xlocate_elem(struct list *l,void *elem,int offset,int length);

/*
 * The same of calling xlocate_node() with offset=0 and length=0
 */
struct list_node *locate_node(struct list *l,void *elem);

/*
 * The same of locate_node, but return the content of the node
 */
void *locate_elem(struct list *l,void *elem);

/*
 * Delete all the elements in the list
 */
void flush_list(struct list *l);

/*
 * Delete every element in the list, and free the memory pointed by all the node data
 */
void destroy_list(struct list *l);

#endif
