
#include "http_simple.h"

static char* g_useragent[] = {
    "Mozilla/5.0 (Windows NT 6.3; WOW64; rv:40.0) Gecko/20100101 Firefox/40.0",
    "Mozilla/5.0 (Windows NT 6.3; WOW64; rv:40.0) Gecko/20100101 Firefox/44.0",
    "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2228.0 Safari/537.36",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/535.11 (KHTML, like Gecko) Ubuntu/11.10 Chromium/27.0.1453.93 Chrome/27.0.1453.93 Safari/537.36",
    "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:35.0) Gecko/20100101 Firefox/35.0",
    "Mozilla/5.0 (compatible; WOW64; MSIE 10.0; Windows NT 6.2)",
    "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US) AppleWebKit/533.20.25 (KHTML, like Gecko) Version/5.0.4 Safari/533.20.27",
    "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.3; Trident/7.0; .NET4.0E; .NET4.0C)",
    "Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko",
    "Mozilla/5.0 (Linux; Android 4.4; Nexus 5 Build/BuildID) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/30.0.0.0 Mobile Safari/537.36",
    "Mozilla/5.0 (iPad; CPU OS 5_0 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9A334 Safari/7534.48.3",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 5_0 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9A334 Safari/7534.48.3",
};

static int g_useragent_index = -1;

typedef struct http_simple_local_data {
    int has_sent_header;
    int has_recv_header;
    char *encode_buffer;
    int host_matched;
    char *recv_buffer;
    int recv_buffer_size;
}http_simple_local_data;

void http_simple_local_data_init(http_simple_local_data* local) {
    local->has_sent_header = 0;
    local->has_recv_header = 0;
    local->encode_buffer = NULL;

    local->recv_buffer = malloc(0);
    local->recv_buffer_size = 0;

    local->host_matched = 0;

    if (g_useragent_index == -1) {
        g_useragent_index = xorshift128plus() % (sizeof(g_useragent) / sizeof(*g_useragent));
    }
}

obfs * http_simple_new_obfs() {
    obfs * self = new_obfs();
    self->l_data = malloc(sizeof(http_simple_local_data));
    http_simple_local_data_init((http_simple_local_data*)self->l_data);
    return self;
}

void http_simple_dispose(obfs *self) {
    http_simple_local_data *local = (http_simple_local_data*)self->l_data;
    if (local->encode_buffer != NULL) {
        free(local->encode_buffer);
        local->encode_buffer = NULL;
    }
    free(local);
    dispose_obfs(self);
}

char http_simple_hex(char c) {
    if (c < 10) return c + '0';
    return c - 10 + 'a';
}

int get_data_from_http_header(char *data, char **outdata) {
    char *delim = "\r\n";
    char *delim_hex = "%";
    int outlength = 0;

    char *buf = *outdata;
    char *p_line;
    p_line = strtok(data, delim);

    //while(p_line)
    {
        char *p_hex;

        p_hex = strtok(p_line, delim_hex);

        while((p_hex = strtok(NULL, delim_hex)))
        {
            char hex = 0;

            if(strlen(p_hex) <= 0)
            {
                continue;
            }

            if(strlen(p_hex) > 2)
            {
                char *c_hex = (char*)malloc(2);
                memcpy(c_hex, p_hex, 2);
                hex = (char)strtol(c_hex, NULL, 16);
                free(c_hex);
            }
            else
            {
                hex = (char)strtol(p_hex, NULL, 16);
            }

            outlength += 1;
            buf = (char*)realloc(buf, outlength);
            buf[outlength - 1] = hex;
        }

        //p_line = strtok(p_line, delim);
    }
    return outlength;
}

void get_host_from_http_header(char *data, char **host) {
    char* data_begin = strstr(data, "Host: ");

    if(data_begin == NULL)
    {
        return;
    }

    data_begin += 6;
    char* data_end = strstr(data_begin, "\r\n");
    char* data_end_port = strstr(data_begin, ":");

    int host_length = 0;

    if(data_end_port != NULL)
    {
        host_length = data_end_port - data_begin;
    }
    else
    {
        host_length = data_end - data_begin;
    }

    if(host_length <= 0)
    {
        return;
    }

    memset(*host, 0x00, 1024);
    memcpy(*host, data_begin, host_length);
}

void http_simple_encode_head(http_simple_local_data *local, char *data, int datalength) {
    if (local->encode_buffer == NULL) {
        local->encode_buffer = (char*)malloc(datalength * 3 + 1);
    }
    int pos = 0;
    for (; pos < datalength; ++pos) {
        local->encode_buffer[pos * 3] = '%';
        local->encode_buffer[pos * 3 + 1] = http_simple_hex(((unsigned char)data[pos] >> 4));
        local->encode_buffer[pos * 3 + 2] = http_simple_hex(data[pos] & 0xF);
    }
    local->encode_buffer[pos * 3] = 0;
}

int http_simple_client_encode(obfs *self, char **pencryptdata, int datalength, size_t* capacity) {
    char *encryptdata = *pencryptdata;
    http_simple_local_data *local = (http_simple_local_data*)self->l_data;
    if (local->has_sent_header) {
        return datalength;
    }
    char hosts[1024];
    char * phost[128];
    int host_num = 0;
    int pos;
    char hostport[128];
    int head_size = self->server.head_len + (xorshift128plus() & 0x3F);
    int outlength;
    char * out_buffer = (char*)malloc(datalength + 2048);
    char * body_buffer = NULL;
    if (head_size > datalength)
        head_size = datalength;
    http_simple_encode_head(local, encryptdata, head_size);
    if (self->server.param && strlen(self->server.param) == 0)
        self->server.param = NULL;
    strncpy(hosts, self->server.param ? self->server.param : self->server.host, sizeof hosts);
    phost[host_num++] = hosts;
    for (pos = 0; hosts[pos]; ++pos) {
        if (hosts[pos] == ',') {
            phost[host_num++] = &hosts[pos + 1];
            hosts[pos] = 0;
        } else if (hosts[pos] == '#') {
            char * body_pointer = &hosts[pos + 1];
            char * p;
            int trans_char = 0;
            p = body_buffer = (char*)malloc(2048);
            for ( ; *body_pointer; ++body_pointer) {
                if (*body_pointer == '\\') {
                    trans_char = 1;
                    continue;
                } else if (*body_pointer == '\n') {
                    *p = '\r';
                    *++p = '\n';
                    continue;
                }
                if (trans_char) {
                    if (*body_pointer == '\\' ) {
                        *p = '\\';
                    } else if (*body_pointer == 'n' ) {
                        *p = '\r';
                        *++p = '\n';
                    } else {
                        *p = '\\';
                        *p = *body_pointer;
                    }
                    trans_char = 0;
                } else {
                    *p = *body_pointer;
                }
                ++p;
            }
            *p = 0;
            hosts[pos] = 0;
            break;
        }
    }
    host_num = xorshift128plus() % host_num;
    if (self->server.port == 80)
        sprintf(hostport, "%s", phost[host_num]);
    else
        sprintf(hostport, "%s:%d", phost[host_num], self->server.port);
    if (body_buffer) {
        sprintf(out_buffer,
            "GET /%s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "%s\r\n\r\n",
            local->encode_buffer,
            hostport,
            body_buffer);
    } else {
        sprintf(out_buffer,
            "GET /%s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: %s\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
            "Accept-Language: en-US,en;q=0.8\r\n"
            "Accept-Encoding: gzip, deflate\r\n"
            "DNT: 1\r\n"
            "Connection: keep-alive\r\n"
            "\r\n",
            local->encode_buffer,
            hostport,
            g_useragent[g_useragent_index]
            );
    }
    //LOGI("http header: %s", out_buffer);
    outlength = strlen(out_buffer);
    memmove(out_buffer + outlength, encryptdata + head_size, datalength - head_size);
    outlength += datalength - head_size;
    local->has_sent_header = 1;
    if (*capacity < outlength) {
        *pencryptdata = (char*)realloc(*pencryptdata, *capacity = outlength * 2);
        encryptdata = *pencryptdata;
    }
    memmove(encryptdata, out_buffer, outlength);
    free(out_buffer);
    if (body_buffer != NULL)
        free(body_buffer);
    if (local->encode_buffer != NULL) {
        free(local->encode_buffer);
        local->encode_buffer = NULL;
    }
    return outlength;
}

int http_simple_server_encode(obfs *self, char **pencryptdata, int datalength, size_t* capacity) {
    char *encryptdata = *pencryptdata;
    http_simple_local_data *local = (http_simple_local_data*)self->l_data;
    if (local->has_sent_header) {
        return datalength;
    }
    int outlength;
    char * out_buffer = (char*)malloc(datalength + 2048);

    time_t now;
    struct tm *tm_now;
    char    datetime[200];

    time(&now);
    tm_now = localtime(&now);
    strftime(datetime, 200, "%a, %d %b %Y %H:%M:%S GMT", tm_now);

    sprintf(out_buffer,
        "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Encoding: gzip\r\nContent-Type: text/html\r\nDate: "
        "%s"
        "\r\nServer: nginx\r\nVary: Accept-Encoding\r\n\r\n",
        datetime);

    outlength = strlen(out_buffer);
    memmove(out_buffer + outlength, encryptdata, datalength);
    outlength += datalength;

    local->has_sent_header = 1;
    if (*capacity < outlength) {
        *pencryptdata = (char*)realloc(*pencryptdata, *capacity = outlength * 2);
        encryptdata = *pencryptdata;
    }
    memmove(encryptdata, out_buffer, outlength);
    free(out_buffer);
    return outlength;
}

int http_simple_client_decode(obfs *self, char **pencryptdata, int datalength, size_t* capacity, int *needsendback) {
    char *encryptdata = *pencryptdata;
    http_simple_local_data *local = (http_simple_local_data*)self->l_data;
    *needsendback = 0;
    if (local->has_recv_header) {
        return datalength;
    }
    char* data_begin = strstr(encryptdata, "\r\n\r\n");
    if (data_begin) {
        int outlength;
        data_begin += 4;
        local->has_recv_header = 1;
        outlength = datalength - (data_begin - encryptdata);
        memmove(encryptdata, data_begin, outlength);
        return outlength;
    } else {
        return 0;
    }
}

int http_simple_server_decode(obfs *self, char **pencryptdata, int datalength, size_t* capacity, int *needsendback) {
    char *encryptdata = *pencryptdata;
    http_simple_local_data *local = (http_simple_local_data*)self->l_data;
    *needsendback = 0;
    if (local->has_recv_header) {
        return datalength;
    }

    if(datalength != 0)
    {
        local->recv_buffer = (char*)realloc(local->recv_buffer, local->recv_buffer_size + datalength);
        memmove(local->recv_buffer + local->recv_buffer_size, encryptdata, datalength);
        local->recv_buffer_size += datalength;

        int outlength = local->recv_buffer_size;
        if (*capacity < outlength) {
            *pencryptdata = (char*)realloc(*pencryptdata, *capacity = outlength * 2);
            encryptdata = *pencryptdata;
        }
        memcpy(encryptdata, local->recv_buffer, local->recv_buffer_size);
    }

    if(local->recv_buffer_size > 10)
    {
        if(strstr(local->recv_buffer, "GET /") == local->recv_buffer || strstr(local->recv_buffer, "POST /") == local->recv_buffer)
        {
            if(local->recv_buffer_size > 65536)
            {
                free(local->recv_buffer);
                local->recv_buffer = malloc(0);
                local->recv_buffer_size = 0;
                local->has_sent_header = 1;
                local->has_recv_header = 1;
                LOGE("http_simple: over size");
                return -1;
            }
        }
        else
        {
            free(local->recv_buffer);
            local->recv_buffer = malloc(0);
            local->recv_buffer_size = 0;
            local->has_sent_header = 1;
            local->has_recv_header = 1;
            LOGE("http_simple: not match begin");
            return -1;
        }
    }
    else
    {
        LOGE("http_simple: too short");
        local->has_sent_header = 1;
        local->has_recv_header = 1;
        return -1;
    }

    char* data_begin = strstr(encryptdata, "\r\n\r\n");
    if (data_begin) {
        int outlength;
        char *ret_buf = (char*)malloc(*capacity);
        memset(ret_buf, 0x00, *capacity);
        int ret_buf_len = 0;
        ret_buf_len = get_data_from_http_header(encryptdata, &ret_buf);

        if (self->server.param && strlen(self->server.param) == 0)
        {
            self->server.param = NULL;
        }
        else
        {
            if(local->host_matched == 0)
            {
                char *host = (char*)malloc(1024);
                get_host_from_http_header(local->recv_buffer, &host);
                char hosts[1024];
                char * phost[128];
                int host_num = 0;
                int pos = 0;
                int is_match = 0;
                char * body_buffer = NULL;
                strncpy(hosts, self->server.param, sizeof hosts);
                phost[host_num++] = hosts;

                for (pos = 0; hosts[pos]; ++pos) {
                    if (hosts[pos] == ',') {
                        phost[host_num++] = &hosts[pos + 1];
                        hosts[pos] = 0;
                    } else if (hosts[pos] == '#') {
                        char * body_pointer = &hosts[pos + 1];
                        char * p;
                        int trans_char = 0;
                        p = body_buffer = (char*)malloc(2048);
                        for ( ; *body_pointer; ++body_pointer) {
                            if (*body_pointer == '\\') {
                                trans_char = 1;
                                continue;
                            } else if (*body_pointer == '\n') {
                                *p = '\r';
                                *++p = '\n';
                                continue;
                            }
                            if (trans_char) {
                                if (*body_pointer == '\\' ) {
                                    *p = '\\';
                                } else if (*body_pointer == 'n' ) {
                                    *p = '\r';
                                    *++p = '\n';
                                } else {
                                    *p = '\\';
                                    *p = *body_pointer;
                                }
                                trans_char = 0;
                            } else {
                                *p = *body_pointer;
                            }
                            ++p;
                        }
                        *p = 0;
                        hosts[pos] = 0;
                        break;
                    }
                }


                for(pos = 0; pos < host_num; pos++)
                {
                    if(strcmp(phost[pos], host) == 0)
                    {
                        is_match = 1;
                        local->host_matched = 1;
                    }
                }

                if(is_match == 0)
                {
                    free(local->recv_buffer);
                    local->recv_buffer = malloc(0);
                    local->recv_buffer_size = 0;
                    local->has_sent_header = 1;
                    local->has_recv_header = 1;
                    LOGE("http_simple: not match host, host: %s", host);
                    return -1;
                }

                free(host);
            }
        }

        if(ret_buf_len <= 0)
        {
            return -1;
        }

        data_begin += 4;
        local->has_recv_header = 1;

        ret_buf = (char*)realloc(ret_buf, ret_buf_len + datalength - (data_begin - encryptdata));
        outlength = ret_buf_len + datalength - (data_begin - encryptdata);

        memcpy(ret_buf + ret_buf_len, data_begin, datalength - (data_begin - encryptdata));

        if (*capacity < outlength) {
            *pencryptdata = (char*)realloc(*pencryptdata, *capacity = outlength * 2);
            encryptdata = *pencryptdata;
        }

        memcpy(encryptdata, ret_buf, outlength);
        free(ret_buf);
        return outlength;
    } else {
        return 0;
    }
}

void boundary(char result[])
{
    char *str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int i,lstr;
    char ss[3] = {0};
    lstr = strlen(str);
    srand((unsigned int)time((time_t *)NULL));
    for(i = 0; i < 32; ++i)
    {
        sprintf(ss, "%c", str[(rand()%lstr)]);
        strcat(result, ss);
    }
}

int http_post_client_encode(obfs *self, char **pencryptdata, int datalength, size_t* capacity) {
    char *encryptdata = *pencryptdata;
    http_simple_local_data *local = (http_simple_local_data*)self->l_data;
    if (local->has_sent_header) {
        return datalength;
    }
    char hosts[1024];
    char * phost[128];
    int host_num = 0;
    int pos;
    char hostport[128];
    int head_size = self->server.head_len + (xorshift128plus() & 0x3F);
    int outlength;
    char * out_buffer = (char*)malloc(datalength + 2048);
    char * body_buffer = NULL;
    if (head_size > datalength)
        head_size = datalength;
    http_simple_encode_head(local, encryptdata, head_size);
    if (self->server.param && strlen(self->server.param) == 0)
        self->server.param = NULL;
    strncpy(hosts, self->server.param ? self->server.param : self->server.host, sizeof hosts);
    phost[host_num++] = hosts;
    for (pos = 0; hosts[pos]; ++pos) {
        if (hosts[pos] == ',') {
            phost[host_num++] = &hosts[pos + 1];
            hosts[pos] = 0;
        } else if (hosts[pos] == '#') {
            char * body_pointer = &hosts[pos + 1];
            char * p;
            int trans_char = 0;
            p = body_buffer = (char*)malloc(2048);
            for ( ; *body_pointer; ++body_pointer) {
                if (*body_pointer == '\\') {
                    trans_char = 1;
                    continue;
                } else if (*body_pointer == '\n') {
                    *p = '\r';
                    *++p = '\n';
                    continue;
                }
                if (trans_char) {
                    if (*body_pointer == '\\' ) {
                        *p = '\\';
                    } else if (*body_pointer == 'n' ) {
                        *p = '\r';
                        *++p = '\n';
                    } else {
                        *p = '\\';
                        *p = *body_pointer;
                    }
                    trans_char = 0;
                } else {
                    *p = *body_pointer;
                }
                ++p;
            }
            *p = 0;
            hosts[pos] = 0;
            break;
        }
    }
    host_num = xorshift128plus() % host_num;
    if (self->server.port == 80)
        sprintf(hostport, "%s", phost[host_num]);
    else
        sprintf(hostport, "%s:%d", phost[host_num], self->server.port);
    if (body_buffer) {
        sprintf(out_buffer,
            "POST /%s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "%s\r\n\r\n",
            local->encode_buffer,
            hostport,
            body_buffer);
    } else {
        char result[33] = {0};
        boundary(result);
        sprintf(out_buffer,
            "POST /%s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: %s\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
            "Accept-Language: en-US,en;q=0.8\r\n"
            "Accept-Encoding: gzip, deflate\r\n"
            "Content-Type: multipart/form-data; boundary=%s\r\n"
            "DNT: 1\r\n"
            "Connection: keep-alive\r\n"
            "\r\n",
            local->encode_buffer,
            hostport,
            g_useragent[g_useragent_index],
            result
            );
    }
    //LOGI("http header: %s", out_buffer);
    outlength = strlen(out_buffer);
    memmove(out_buffer + outlength, encryptdata + head_size, datalength - head_size);
    outlength += datalength - head_size;
    local->has_sent_header = 1;
    if (*capacity < outlength) {
        *pencryptdata = (char*)realloc(*pencryptdata, *capacity = outlength * 2);
        encryptdata = *pencryptdata;
    }
    memmove(encryptdata, out_buffer, outlength);
    free(out_buffer);
    if (body_buffer != NULL)
        free(body_buffer);
    if (local->encode_buffer != NULL) {
        free(local->encode_buffer);
        local->encode_buffer = NULL;
    }
    return outlength;
}
