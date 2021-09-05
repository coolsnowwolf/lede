/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include "ngx_map.h"


static void
ngx_map_rbtree_insert_value(ngx_rbtree_node_t *temp, ngx_rbtree_node_t *node,
        ngx_rbtree_node_t *sentinel)
{
    ngx_rbtree_node_t         **p;
    intptr_t                   *raw_key, *raw_key_temp;
    ngx_map_t                 **map;

    for (;;) {

        if (node->key < temp->key) {
            p = &temp->left;
        } else if (node->key > temp->key) {
            p = &temp->right;
        } else {
            raw_key = (intptr_t *)((char *) node
                                        + offsetof(ngx_map_node_t, raw_key));
            raw_key_temp = (intptr_t *)((char *) temp
                                        + offsetof(ngx_map_node_t, raw_key));
            map = (ngx_map_t **)((char *) node + offsetof(ngx_map_node_t, map));

            switch ((*map)->cmp(*raw_key, *raw_key_temp)) {
            case -1:
                p = &temp->left;
                break;
            case 1:
                p = &temp->right;
                break;
            default:     /* key is duplicate */
                return;
            }
        }

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ngx_rbt_red(node);
}

void
ngx_map_init(ngx_map_t *map, ngx_map_hash_pt hash, ngx_cmp_pt cmp)
{
    ngx_rbtree_init(&map->rbtree, &map->sentinel, ngx_map_rbtree_insert_value);
    map->hash = hash;
    map->cmp = cmp;
}

ngx_map_node_t *
ngx_map_begin(ngx_map_t *map)
{
    ngx_rbtree_node_t          *p;

    if (ngx_map_empty(map)) {
        return NULL;
    }

    p = map->rbtree.root;

    for (;;) {
        if (p->left == map->rbtree.sentinel) {
            break;
        }

        p = p->left;
    }

    return (ngx_map_node_t *) p;
}

ngx_map_node_t *
ngx_map_rbegin(ngx_map_t *map)
{
    ngx_rbtree_node_t          *p;

    if (ngx_map_empty(map)) {
        return NULL;
    }

    p = map->rbtree.root;

    for (;;) {
        if (p->right == map->rbtree.sentinel) {
            break;
        }

        p = p->right;
    }

    return (ngx_map_node_t *) p;
}

ngx_map_node_t *
ngx_map_next(ngx_map_node_t *n)
{
    ngx_map_t                  *map;
    ngx_rbtree_node_t          *p, *top;

    map = n->map;
    p = &n->rn;
    top = NULL;

    if (p->right != map->rbtree.sentinel) {
        // current node has right subtree
        top = p->right;
    } else {
        for (;;) {
            if (p == map->rbtree.root) {
                return NULL;
            }

            if (p->parent->left == p) {
                return (ngx_map_node_t *) p->parent;
            }

            // p->parent->right == p
            p = p->parent;
        }
    }

    // get the mininum node
    p = top;
    for (;;) {
        if (p->left == map->rbtree.sentinel) {
            break;
        }

        p = p->left;
    }

    return (ngx_map_node_t *) p;
}

ngx_map_node_t *
ngx_map_prev(ngx_map_node_t *n)
{
    ngx_map_t                  *map;
    ngx_rbtree_node_t          *p, *top;

    map = n->map;
    p = &n->rn;
    top = NULL;

    if (p->left != map->rbtree.sentinel) {
        // current node has left subtree
        top = p->left;
    } else {
        for (;;) {
            if (p == map->rbtree.root) {
                return NULL;
            }

            if (p->parent->right == p) {
                return (ngx_map_node_t *) p->parent;
            }

            // p->parent->left == p
            p = p->parent;
        }
    }

    // get the maximum node
    p = top;
    for (;;) {
        if (p->right == map->rbtree.sentinel) {
            break;
        }

        p = p->right;
    }

    return (ngx_map_node_t *) p;
}

void
ngx_map_insert(ngx_map_t *map, ngx_map_node_t *node, ngx_flag_t covered)
{
    ngx_map_node_t             *n;

    node->rn.key = map->hash(node->raw_key);
    node->map = map;

    n = ngx_map_find(map, node->raw_key);
    if (n == NULL) {
        ngx_rbtree_insert(&map->rbtree, &node->rn);
    } else if (covered) {
        ngx_map_delete(map, node->raw_key);
        ngx_rbtree_insert(&map->rbtree, &node->rn);
    }
}

void
ngx_map_delete(ngx_map_t *map, intptr_t key)
{
    ngx_map_node_t             *node;

    node = ngx_map_find(map, key);
    if (node) {
        ngx_rbtree_delete(&map->rbtree, &node->rn);
    }
}

ngx_map_node_t *
ngx_map_find(ngx_map_t *map, intptr_t key)
{
    ngx_rbtree_node_t          *p;
    ngx_rbtree_key_t            k;
    intptr_t                   *key_temp;

    if (ngx_map_empty(map)) {
        return NULL;
    }

    k = map->hash(key);
    p = map->rbtree.root;

    for (;;) {
        if (k < p->key) {
            p = p->left;
        } else if (k > p->key) {
            p = p->right;
        } else {
            key_temp = (intptr_t *)((char *) p
                     + offsetof(ngx_map_node_t, raw_key));

            switch (map->cmp(key, *key_temp)) {
            case -1:
                p = p->left;
                break;
            case 1:
                p = p->right;
                break;
            case 0:
                return (ngx_map_node_t *) p;
            }
        }

        if (p == map->rbtree.sentinel) {
            return NULL;
        }
    }
}


/* ngx_str_t */
ngx_rbtree_key_t
ngx_map_hash_str(intptr_t key)
{
    ngx_str_t                  *k;

    k = (ngx_str_t *) key;

    return ngx_hash_key(k->data, k->len);
}

int
ngx_cmp_str(intptr_t key1, intptr_t key2)
{
    ngx_str_t                  *k1, *k2;
    ngx_int_t                   rc;

    k1 = (ngx_str_t *) key1;
    k2 = (ngx_str_t *) key2;

    rc = ngx_memn2cmp(k1->data, k2->data, k1->len, k2->len);

    if (rc < 0) {
        return -1;
    } else if (rc > 0) {
        return 1;
    } else {
        return 0;
    }
}

/* ngx_uint_t */
ngx_rbtree_key_t
ngx_map_hash_uint(intptr_t key)
{
    return (ngx_rbtree_key_t) key;
}

int
ngx_cmp_uint(intptr_t key1, intptr_t key2)
{
    if ((ngx_uint_t) key1 < (ngx_uint_t) key2) {
        return -1;
    } else if ((ngx_uint_t) key1 > (ngx_uint_t) key2) {
        return 1;
    } else {
        return 0;
    }
}

/* ngx_int_t */
ngx_rbtree_key_t
ngx_map_hash_int(intptr_t key)
{
    return (ngx_rbtree_key_t) key;
}

int
ngx_cmp_int(intptr_t key1, intptr_t key2)
{
    if ((ngx_int_t) key1 < (ngx_int_t) key2) {
        return -1;
    } else if ((ngx_int_t) key1 > (ngx_int_t) key2) {
        return 1;
    } else {
        return 0;
    }
}
