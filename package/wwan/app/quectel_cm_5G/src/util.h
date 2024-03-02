/**  
  @file
  util.h

  @brief
  This file provides the definitions, and declares some common APIs for list-algorithm.

 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stddef.h>
#include <glob.h>

struct listnode
{
    struct listnode *next;
    struct listnode *prev;
};

#define node_to_item(node, container, member) \
    (container *) (((char*) (node)) - offsetof(container, member))

#define list_declare(name) \
    struct listnode name = { \
        .next = &name, \
        .prev = &name, \
    }

#define list_for_each(node, list) \
    for (node = (list)->next; node != (list); node = node->next)

#define list_for_each_reverse(node, list) \
    for (node = (list)->prev; node != (list); node = node->prev)

void list_init(struct listnode *list);
void list_add_tail(struct listnode *list, struct listnode *item);
void list_add_head(struct listnode *head, struct listnode *item);
void list_remove(struct listnode *item);

#define list_empty(list) ((list) == (list)->next)
#define list_head(list) ((list)->next)
#define list_tail(list) ((list)->prev)

int epoll_register(int  epoll_fd, int  fd, unsigned int events);
int epoll_deregister(int  epoll_fd, int  fd);
const char * get_time(void);
unsigned long clock_msec(void);
pid_t getpid_by_pdp(int, const char*);

#endif
