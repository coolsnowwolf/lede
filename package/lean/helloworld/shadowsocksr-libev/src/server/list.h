#ifndef LIST_H_H
#define LIST_H_H

#include <stdio.h>
#include <malloc.h>
#include <string.h>

typedef struct clist *List;

typedef int (*compare)(void *ndata, void *data);
typedef void (*dofunc)(void *ndata);

typedef int (*lpf0)(List l, void *data);
typedef int (*lpf1)(List l, void *data, compare pfunc);
typedef List (*lpf2)(List l);
typedef void (*lpf3)(List l);
typedef void (*lpf4)(List l, dofunc pfunc);
typedef int (*lpf5)(List l, unsigned int index, void *new_data);
typedef void (*lpf6)(List l, compare pfunc);
typedef int (*lpf7)(List l, unsigned int index);

typedef struct cnode
{
    void *data;
    struct cnode *next;
}node, *Node;

typedef struct clist
{
    Node head;
    Node tail;
    unsigned int size;
    unsigned int data_size;
    lpf0 add_back;
    lpf0 add_front;
    lpf1 delete_node;
    lpf1 have_same;
    lpf0 have_same_cmp;
    lpf4 foreach;
    lpf3 clear;
    lpf2 destroy;
    lpf5 modify_at;
    lpf6 sort;
    lpf7 delete_at;
}list;

//初始化链表
List list_init(unsigned int data_size);
int  list_add_back(List l, void *data);
int  list_add_front(List l, void *data);
int  list_delete_node(List l, void *data, compare pfunc);
int  list_delete_at(List l, unsigned int index);
int  list_modify_at(List l, unsigned int index, void *new_data);
int  list_have_same(List l, void *data, compare pfunc);
int  list_have_same_cmp(List l, void *data);
void list_foreach(List l, dofunc doit);
void list_sort(List l, compare pfunc);
void list_clear(List l);
//释放链表
List list_destroy(List l);
#endif
