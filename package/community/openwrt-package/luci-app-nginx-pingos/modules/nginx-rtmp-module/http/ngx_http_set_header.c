#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct ngx_http_header_val_s  ngx_http_header_val_t;

typedef ngx_int_t (*ngx_http_set_header_pt)(ngx_http_request_t *r,
    ngx_http_header_val_t *hv, ngx_str_t *value);

struct ngx_http_header_val_s {
    ngx_http_complex_value_t                value;
    ngx_uint_t                              hash;
    ngx_str_t                               key;
    ngx_http_set_header_pt                  handler;
    ngx_uint_t                              offset;
};

typedef struct {
    ngx_str_t                               name;
    ngx_uint_t                              offset;
    ngx_http_set_header_pt                  handler;

} ngx_http_set_header_t;

//TODO need fill all header set in future
/* for header has no quick link in ngx_http_headers_out_t */
static ngx_int_t ngx_http_set_header_out_other(ngx_http_request_t *r,
    ngx_http_header_val_t *hv, ngx_str_t *value);
/* for header has quick link like ngx_table_elt_t* in ngx_http_headers_out_t */
static ngx_int_t ngx_http_set_header_out_builtin(ngx_http_request_t *r,
    ngx_http_header_val_t *hv, ngx_str_t *value);
/* for header has quick link like ngx_array_t in ngx_http_headers_out_t */
static ngx_int_t ngx_http_set_header_out_builtin_multi(ngx_http_request_t *r,
    ngx_http_header_val_t *hv, ngx_str_t *value);
/* for header has quick link like ngx_table_elt_t* and
 * other attribute in ngx_http_headers_out_t defined below */
static ngx_int_t ngx_http_set_header_out_content_type(ngx_http_request_t *r,
    ngx_http_header_val_t *hv, ngx_str_t *value);

static ngx_http_set_header_t  ngx_http_set_header_out_handlers[] = {

    { ngx_string("Server"),
                 offsetof(ngx_http_headers_out_t, server),
                 ngx_http_set_header_out_builtin },

    { ngx_string("Date"),
                 offsetof(ngx_http_headers_out_t, date),
                 ngx_http_set_header_out_builtin },

    { ngx_string("Content-Type"),
                 offsetof(ngx_http_headers_out_t, content_type),
                 ngx_http_set_header_out_content_type },

    { ngx_string("Cache-Control"),
                 offsetof(ngx_http_headers_out_t, cache_control),
                 ngx_http_set_header_out_builtin_multi },

    { ngx_null_string, 0, ngx_http_set_header_out_other }
};

static ngx_int_t
ngx_http_set_header_out_helper(ngx_http_request_t *r,
    ngx_http_header_val_t *hv, ngx_str_t *value, unsigned no_create)
{
    ngx_table_elt_t             *h;
    ngx_list_part_t             *part;
    ngx_uint_t                   i;

    part = &r->headers_out.headers.part;
    h = part->elts;

    for (i = 0; /* void */; ++i) {

        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }

            part = part->next;
            h = part->elts;
            i = 0;
        }

        if (h[i].hash != 0
            && h[i].key.len == hv->key.len
            && ngx_strncasecmp(hv->key.data, h[i].key.data, h[i].key.len) == 0)
            /* header has been set */
        {
            h[i].value = *value;
            if (value->len == 0) { /* if value is empty, remove header */
                h[i].hash = 0;
            } else {
                h[i].hash = hv->hash;
            }

            return NGX_OK;
        }
    }

    if (no_create && value->len == 0) { /* set header to empty but header cannot found */
        return NGX_OK;
    }

    /* header has not been set, create it */

    h = ngx_list_push(&r->headers_out.headers);

    if (h == NULL) {
        return NGX_ERROR;
    }

    h->hash = hv->hash;
    h->key = hv->key;
    h->value = *value;

    h->lowcase_key = ngx_pnalloc(r->pool, h->key.len);
    if (h->lowcase_key == NULL) {
        return NGX_ERROR;
    }

    ngx_strlow(h->lowcase_key, h->key.data, h->key.len);

    return NGX_OK;
}

static ngx_int_t
ngx_http_set_header_out_other(ngx_http_request_t * r,
    ngx_http_header_val_t * hv,ngx_str_t * value)
{
    return ngx_http_set_header_out_helper(r, hv, value, 0);
}


static ngx_int_t
ngx_http_set_header_out_builtin(ngx_http_request_t *r,
    ngx_http_header_val_t *hv, ngx_str_t *value)
{
    ngx_table_elt_t  *h, **old;

    if (hv->offset) {
        old = (ngx_table_elt_t **) ((char *) &r->headers_out + hv->offset);
    } else {
        old = NULL;
    }

    if (old == NULL || *old == NULL) {
    /* user should use ngx_http_set_header_out_other but use this func to set header */
        //TODO
        return ngx_http_set_header_out_helper(r, hv, value, 0);
    }

    h = *old;

    h->value = *value;
    if (value->len == 0) { /* if value is empty, remove header */

        h->hash = 0;
        return NGX_OK;
    }

    h->hash = hv->hash;
    h->key = hv->key;

    return NGX_OK;
}

static ngx_int_t
ngx_http_set_header_out_builtin_multi(ngx_http_request_t *r,
    ngx_http_header_val_t *hv, ngx_str_t *value)
{
    ngx_array_t      *pa;
    ngx_table_elt_t  *ho, **ph;
    ngx_uint_t        i;

    pa = (ngx_array_t *) ((char *) &r->headers_out + hv->offset);

    if (pa->elts == NULL) {
        if (ngx_array_init(pa, r->pool, 2, sizeof(ngx_table_elt_t *))
            != NGX_OK)
        {
            return NGX_ERROR;
        }
    }

    if (pa->nelts > 0) {
        ph = pa->elts;
        for (i = 1; i < pa->nelts; i++) { /* clear old value */
            ph[i]->hash = 0;
            ph[i]->value.len = 0;
        }

        ph[0]->value = *value;

        if (value->len == 0) {
            ph[0]->hash = 0;
        } else {
            ph[0]->hash = hv->hash;
        }

        return NGX_OK;
    }

    /* header does not set */
    ph = ngx_array_push(pa);
    if (ph == NULL) {
        return NGX_ERROR;
    }

    ho = ngx_list_push(&r->headers_out.headers);
    if (ho == NULL) {
        return NGX_ERROR;
    }

    ho->value = *value;
    ho->hash = hv->hash;
    ho->key = hv->key;
    *ph = ho;

    return NGX_OK;
}

static ngx_int_t
ngx_http_set_header_out_content_type(ngx_http_request_t *r,
    ngx_http_header_val_t *hv, ngx_str_t *value)
{
    ngx_uint_t          i;

    r->headers_out.content_type_len = value->len;

    for (i = 0; i < value->len; i++) {
        if (value->data[i] == ';') {
            r->headers_out.content_type_len = i;
            break;
        }
    }

    r->headers_out.content_type = *value;
    r->headers_out.content_type_hash = hv->hash;
    r->headers_out.content_type_lowcase = NULL;

    value->len = 0;

    return ngx_http_set_header_out_helper(r, hv, value, 1);
}

ngx_int_t
ngx_http_set_header_out(ngx_http_request_t *r, ngx_str_t *key, ngx_str_t *value)
{
    ngx_http_header_val_t           hv;
    ngx_http_set_header_t          *handlers = ngx_http_set_header_out_handlers;
    ngx_uint_t                      i;
    ngx_str_t                       v;

    hv.hash = ngx_hash_key_lc(key->data, key->len);
    hv.key = *key;

    hv.offset = 0;
    hv.handler = NULL;

    for (i = 0; handlers[i].name.len; ++i) {
        if (hv.key.len != handlers[i].name.len
            || ngx_strncasecmp(hv.key.data, handlers[i].name.data,
                               handlers[i].name.len) != 0) {
            continue;
        }

        /* match handler */
        hv.offset = handlers[i].offset;
        hv.handler = handlers[i].handler;

        break;
    }

    if (handlers[i].name.len == 0 && handlers[i].handler) { /* if not matched, use ngx_http_set_header as default*/
        hv.offset = handlers[i].offset;
        hv.handler = handlers[i].handler;
    }

    v = *value;

    return hv.handler(r, &hv, &v);
}
