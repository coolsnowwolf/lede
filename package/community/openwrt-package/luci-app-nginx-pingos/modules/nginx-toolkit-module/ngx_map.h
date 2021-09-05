/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_MAP_H_INCLUDED_
#define _NGX_MAP_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * key: key for map node
 */
typedef ngx_rbtree_key_t (* ngx_map_hash_pt)(intptr_t key);

/*
 * if key1 < key2, return -1
 * if key1 = key2, return 0
 * if key1 > key2, return 1
 */
typedef int (* ngx_cmp_pt)(intptr_t key1, intptr_t key2);


/* ngx_str_t */
ngx_rbtree_key_t ngx_map_hash_str(intptr_t key);
int ngx_cmp_str(intptr_t key1, intptr_t key2);

/* ngx_uint_t */
ngx_rbtree_key_t ngx_map_hash_uint(intptr_t key);
int ngx_cmp_uint(intptr_t key1, intptr_t key2);

/* ngx_int_t */
ngx_rbtree_key_t ngx_map_hash_int(intptr_t key);
int ngx_cmp_int(intptr_t key1, intptr_t key2);


typedef struct {
    ngx_rbtree_t                rbtree;
    ngx_rbtree_node_t           sentinel;
    ngx_map_hash_pt             hash;
    ngx_cmp_pt                  cmp;
} ngx_map_t;

typedef struct {
    ngx_rbtree_node_t           rn;
    intptr_t                    raw_key;
    ngx_map_t                  *map;
} ngx_map_node_t;


/*
 * return value:
 *      None
 * paras:
 *      map : map for initial
 *      hash: hash func for calc key's hash
 *      cmp : cmp func for cmp two keys
 */
void ngx_map_init(ngx_map_t *map, ngx_map_hash_pt hash, ngx_cmp_pt cmp);

/*
 * return value:
 *      if map is empty return 1, else return 0
 */
#define ngx_map_empty(map) (map->rbtree.root == map->rbtree.sentinel)

/*
 * return value:
 *      the mininum key map node, if map is empty, return NULL
 * paras:
 *      map: map for operate
 */
ngx_map_node_t *ngx_map_begin(ngx_map_t *map);

/*
 * return value:
 *      the maximum key map node, if map is empty, return NULL
 * paras:
 *      map: map for operate
 */
ngx_map_node_t *ngx_map_rbegin(ngx_map_t *map);

/*
 * return value:
 *      the next bigger key map node, if none, return NULL
 * paras:
 *      n  : current node
 */
ngx_map_node_t *ngx_map_next(ngx_map_node_t *n);

/*
 * return value:
 *      the next smaller key map node, if none, return NULL
 * paras:
 *      n  : current node
 */
ngx_map_node_t *ngx_map_prev(ngx_map_node_t *n);

/*
 * return value:
 *      None
 * paras:
 *      map    : map for operate
 *      node   : map node for inserting into map
 *      covered: 1 for covered if key is same, 0 do nothing if key is same
 */
void ngx_map_insert(ngx_map_t *map, ngx_map_node_t *node, ngx_flag_t covered);

/*
 * return value:
 *      None
 * paras:
 *      map: map for operate
 *      key: map node key for deleting from map
 */
void ngx_map_delete(ngx_map_t *map, intptr_t key);

/*
 * return value:
 *      node in map searching by key, NULL for not found
 * paras:
 *      map: map for operate
 *      key: node key for searching
 */
ngx_map_node_t *ngx_map_find(ngx_map_t *map, intptr_t key);


#endif
