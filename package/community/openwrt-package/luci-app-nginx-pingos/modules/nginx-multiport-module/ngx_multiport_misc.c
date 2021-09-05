#include <ngx_config.h>
#include <ngx_core.h>


static ngx_int_t
ngx_multiport_get_port_unix(ngx_pool_t *pool, ngx_str_t *port,
        ngx_str_t *multiport, ngx_int_t pslot)
{
#if (NGX_HAVE_UNIX_DOMAIN)
    u_char                         *p;
    size_t                          len;

    len = multiport->len + 5; /* unix:/path -> unix:/path.127\0 */
    port->data = ngx_pcalloc(pool, len);
    if (port->data == NULL) {
        return NGX_ERROR;
    }

    p = ngx_snprintf(port->data, len, "%V.%i", multiport, pslot);
    *p = 0;
    port->len = p - port->data;

    return NGX_OK;
#else
    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                  "the unix domain sockets not support");
    return NGX_ERROR;
#endif
}

static ngx_int_t
ngx_multiport_get_port_inet6(ngx_pool_t *pool, ngx_str_t *port,
        ngx_str_t *multiport, ngx_int_t pslot)
{
#if (NGX_HAVE_INET6)
    u_char                         *p, *last;
    ngx_str_t                       addr;
    size_t                          len;
    ngx_int_t                       n;

    last = multiport->data + multiport->len;
    p = ngx_strlchr(multiport->data, last, ']');

    if (p == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "invalid INET6 host");
        return NGX_ERROR;
    }

    ++p;
    if (p == last || *p != ':') {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "no INET6 port");
        return NGX_ERROR;
    }

    ++p;
    addr.data = multiport->data;
    addr.len = p - multiport->data;

    len = last - p;
    n = ngx_atoi(p, len);

    /* 65408 + 127 = 65535, pslot in [0, 127] */
    if (n < 1 || n + pslot > 65408) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "invalid INET6 port");
        return NGX_ERROR;
    }
    n += pslot;

    len = multiport->len + 3;   /* [::]:1 -> [::]:128\0 */
    port->data = ngx_pcalloc(pool, len);
    if (port->data == NULL) {
        return NGX_ERROR;
    }

    p = port->data;
    p = ngx_snprintf(p, len, "%V%i", &addr, n);
    port->len = p - port->data;

    return NGX_OK;
#else
    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
            "the INET6 sockets not support");
    return NGX_ERROR;
#endif
}

static ngx_int_t
ngx_multiport_get_port_inet(ngx_pool_t *pool, ngx_str_t *port,
        ngx_str_t *multiport, ngx_int_t pslot)
{
    u_char                         *p, *last;
    ngx_str_t                       addr;
    size_t                          len;
    ngx_int_t                       n;

    last = multiport->data + multiport->len;
    p = ngx_strlchr(multiport->data, last, ':');

    if (p == NULL) { /* port */
        p = multiport->data;
        addr.len = 0;
    } else { /* host:port */
        ++p;
        if (p == last) {
            ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "no port");
            return NGX_ERROR;
        }

        addr.data = multiport->data;
        addr.len = p - multiport->data;
    }

    len = last - p;
    n = ngx_atoi(p, len);

    /* 65408 + 127 = 65535, pslot in [0, 127] */
    if (n < 1 || n + pslot > 65408) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "invalid port");
        return NGX_ERROR;
    }
    n += pslot;

    len = multiport->len + 3;   /* 127.0.0.1:1 -> 127.0.0.1:128\0 */
    port->data = ngx_pcalloc(pool, len);
    if (port->data == NULL) {
        return NGX_ERROR;
    }

    p = port->data;
    if (addr.len == 0) {
        p = ngx_snprintf(p, len, "%i", n);
    } else {
        p = ngx_snprintf(p, len, "%V%i", &addr, n);
    }
    port->len = p - port->data;

    return NGX_OK;
}

ngx_int_t
ngx_multiport_get_port(ngx_pool_t *pool, ngx_str_t *port,
        ngx_str_t *multiport, ngx_int_t pslot)
{
    u_char                         *p;
    size_t                          len;

    p = multiport->data;
    len = multiport->len;

    if (pslot < 0 || pslot > 127) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "invalid pslot: %i",
                pslot);
        return NGX_ERROR;
    }

    if (len >= 5 && ngx_strncasecmp(p, (u_char *) "unix:", 5) == 0) {
        return ngx_multiport_get_port_unix(pool, port, multiport, pslot);
    }

    if (len && p[0] == '[') {
        return ngx_multiport_get_port_inet6(pool, port, multiport, pslot);
    }

    return ngx_multiport_get_port_inet(pool, port, multiport, pslot);
}
