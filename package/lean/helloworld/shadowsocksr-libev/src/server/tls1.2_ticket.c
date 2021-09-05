
#include "tls1.2_ticket.h"
#include "list.c"

typedef struct tls12_ticket_auth_global_data {
    uint8_t local_client_id[32];
    List client_data;
    time_t startup_time;
}tls12_ticket_auth_global_data;

typedef struct tls12_ticket_auth_local_data {
    int handshake_status;
    char *send_buffer;
    int send_buffer_size;
    char *recv_buffer;
    int recv_buffer_size;
}tls12_ticket_auth_local_data;

void tls12_ticket_auth_local_data_init(tls12_ticket_auth_local_data* local) {
    local->handshake_status = 0;
    local->send_buffer = malloc(0);
    local->send_buffer_size = 0;
    local->recv_buffer = malloc(0);
    local->recv_buffer_size = 0;
}

void * tls12_ticket_auth_init_data() {
    tls12_ticket_auth_global_data *global = (tls12_ticket_auth_global_data*)malloc(sizeof(tls12_ticket_auth_global_data));
    rand_bytes(global->local_client_id, 32);
    global->client_data = list_init(22);
    global->startup_time = time(NULL);
    return global;
}

obfs * tls12_ticket_auth_new_obfs() {
    obfs * self = new_obfs();
    self->l_data = malloc(sizeof(tls12_ticket_auth_local_data));
    tls12_ticket_auth_local_data_init((tls12_ticket_auth_local_data*)self->l_data);
    return self;
}

void tls12_ticket_auth_dispose(obfs *self) {
    tls12_ticket_auth_local_data *local = (tls12_ticket_auth_local_data*)self->l_data;
    if (local->send_buffer != NULL) {
        free(local->send_buffer);
        local->send_buffer = NULL;
    }
    if (local->recv_buffer != NULL) {
        free(local->recv_buffer);
        local->recv_buffer = NULL;
    }
    free(local);
    dispose_obfs(self);
}

int tls12_ticket_pack_auth_data(tls12_ticket_auth_global_data *global, server_info *server, char *outdata) {
    int out_size = 32;
    time_t t = time(NULL);
    outdata[0] = t >> 24;
    outdata[1] = t >> 16;
    outdata[2] = t >> 8;
    outdata[3] = t;
    rand_bytes((uint8_t*)outdata + 4, 18);

    uint8_t *key = (uint8_t*)malloc(server->key_len + 32);
    char hash[ONETIMEAUTH_BYTES * 2];
    memcpy(key, server->key, server->key_len);
    memcpy(key + server->key_len, global->local_client_id, 32);
    ss_sha1_hmac_with_key(hash, outdata, out_size - OBFS_HMAC_SHA1_LEN, key, server->key_len + 32);
    free(key);
    memcpy(outdata + out_size - OBFS_HMAC_SHA1_LEN, hash, OBFS_HMAC_SHA1_LEN);
    return out_size;
}

void tls12_ticket_auth_pack_data(char *encryptdata, int datalength, int start, int len, char *out_buffer, int outlength) {
    out_buffer[outlength] = 0x17;
    out_buffer[outlength + 1] = 0x3;
    out_buffer[outlength + 2] = 0x3;
    out_buffer[outlength + 3] = len >> 8;
    out_buffer[outlength + 4] = len;
    memcpy(out_buffer + outlength + 5, encryptdata + start, len);
}

int tls12_ticket_auth_client_encode(obfs *self, char **pencryptdata, int datalength, size_t* capacity) {
    char *encryptdata = *pencryptdata;
    tls12_ticket_auth_local_data *local = (tls12_ticket_auth_local_data*)self->l_data;
    tls12_ticket_auth_global_data *global = (tls12_ticket_auth_global_data*)self->server.g_data;
    char * out_buffer = NULL;

    if (local->handshake_status == 8) {
        if (datalength < 1024) {
            if (*capacity < datalength + 5) {
                *pencryptdata = (char*)realloc(*pencryptdata, *capacity = (datalength + 5) * 2);
                encryptdata = *pencryptdata;
            }
            memmove(encryptdata + 5, encryptdata, datalength);
            encryptdata[0] = 0x17;
            encryptdata[1] = 0x3;
            encryptdata[2] = 0x3;
            encryptdata[3] = datalength >> 8;
            encryptdata[4] = datalength;
            return datalength + 5;
        } else {
            out_buffer = (char*)malloc(datalength + 2048);
            int start = 0;
            int outlength = 0;
            int len;
            while (datalength - start > 2048) {
                len = xorshift128plus() % 4096 + 100;
                if (len > datalength - start)
                    len = datalength - start;
                tls12_ticket_auth_pack_data(encryptdata, datalength, start, len, out_buffer, outlength);
                outlength += len + 5;
                start += len;
            }
            if (datalength - start > 0) {
                len = datalength - start;
                tls12_ticket_auth_pack_data(encryptdata, datalength, start, len, out_buffer, outlength);
                outlength += len + 5;
            }
            if (*capacity < outlength) {
                *pencryptdata = (char*)realloc(*pencryptdata, *capacity = outlength * 2);
                encryptdata = *pencryptdata;
            }
            memcpy(encryptdata, out_buffer, outlength);
            free(out_buffer);
            return outlength;
        }
    }
    local->send_buffer = (char*)realloc(local->send_buffer, local->send_buffer_size + datalength + 5);
    memcpy(local->send_buffer + local->send_buffer_size + 5, encryptdata, datalength);
    local->send_buffer[local->send_buffer_size] = 0x17;
    local->send_buffer[local->send_buffer_size + 1] = 0x3;
    local->send_buffer[local->send_buffer_size + 2] = 0x3;
    local->send_buffer[local->send_buffer_size + 3] = datalength >> 8;
    local->send_buffer[local->send_buffer_size + 4] = datalength;
    local->send_buffer_size += datalength + 5;

    if (local->handshake_status == 0) {
#define CSTR_DECL(name, len, str) const char* name = str; const int len = sizeof(str) - 1;
        CSTR_DECL(tls_data0, tls_data0_len, "\x00\x1c\xc0\x2b\xc0\x2f\xcc\xa9\xcc\xa8\xcc\x14\xcc\x13\xc0\x0a\xc0\x14\xc0\x09\xc0\x13\x00\x9c\x00\x35\x00\x2f\x00\x0a\x01\x00"
                );
        CSTR_DECL(tls_data1, tls_data1_len, "\xff\x01\x00\x01\x00"
                );
        CSTR_DECL(tls_data2, tls_data2_len, "\x00\x17\x00\x00\x00\x23\x00\xd0");
        CSTR_DECL(tls_data3, tls_data3_len, "\x00\x0d\x00\x16\x00\x14\x06\x01\x06\x03\x05\x01\x05\x03\x04\x01\x04\x03\x03\x01\x03\x03\x02\x01\x02\x03\x00\x05\x00\x05\x01\x00\x00\x00\x00\x00\x12\x00\x00\x75\x50\x00\x00\x00\x0b\x00\x02\x01\x00\x00\x0a\x00\x06\x00\x04\x00\x17\x00\x18"
                //"00150066000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" // padding
                );
        uint8_t tls_data[2048];
        int tls_data_len = 0;
        memcpy(tls_data, tls_data1, tls_data1_len);
        tls_data_len += tls_data1_len;

        char hosts[1024];
        char * phost[128];
        int host_num = 0;
        int pos;

        char sni[256] = {0};
        if (self->server.param && strlen(self->server.param) == 0)
            self->server.param = NULL;
        strncpy(hosts, self->server.param ? self->server.param : self->server.host, sizeof hosts);
        phost[host_num++] = hosts;
        for (pos = 0; hosts[pos]; ++pos) {
            if (hosts[pos] == ',') {
                phost[host_num++] = &hosts[pos + 1];
            }
        }
        host_num = xorshift128plus() % host_num;

        sprintf(sni, "%s", phost[host_num]);
        int sni_len = strlen(sni);
        if (sni_len > 0 && sni[sni_len - 1] >= '0' && sni[sni_len - 1] <= '9')
            sni_len = 0;
        tls_data[tls_data_len] = '\0';
        tls_data[tls_data_len + 1] = '\0';
        tls_data[tls_data_len + 2] = (sni_len + 5) >> 8;
        tls_data[tls_data_len + 3] = (sni_len + 5);
        tls_data[tls_data_len + 4] = (sni_len + 3) >> 8;
        tls_data[tls_data_len + 5] = (sni_len + 3);
        tls_data[tls_data_len + 6] = '\0';
        tls_data[tls_data_len + 7] = sni_len >> 8;
        tls_data[tls_data_len + 8] = sni_len;
        memcpy(tls_data + tls_data_len + 9, sni, sni_len);
        tls_data_len += 9 + sni_len;
        memcpy(tls_data + tls_data_len, tls_data2, tls_data2_len);
        tls_data_len += tls_data2_len;
        rand_bytes(tls_data + tls_data_len, 208);
        tls_data_len += 208;
        memcpy(tls_data + tls_data_len, tls_data3, tls_data3_len);
        tls_data_len += tls_data3_len;

        datalength = 11 + 32 + 1 + 32 + tls_data0_len + 2 + tls_data_len;
        out_buffer = (char*)malloc(datalength);
        char *pdata = out_buffer + datalength - tls_data_len;
        int len = tls_data_len;
        memcpy(pdata, tls_data, tls_data_len);
        pdata[-1] = tls_data_len;
        pdata[-2] = tls_data_len >> 8;
        pdata -= 2; len += 2;
        memcpy(pdata - tls_data0_len, tls_data0, tls_data0_len);
        pdata -= tls_data0_len; len += tls_data0_len;
        memcpy(pdata - 32, global->local_client_id, 32);
        pdata -= 32; len += 32;
        pdata[-1] = 0x20;
        pdata -= 1; len += 1;
        tls12_ticket_pack_auth_data(global, &self->server, pdata - 32);
        pdata -= 32; len += 32;
        pdata[-1] = 0x3;
        pdata[-2] = 0x3; // tls version
        pdata -= 2; len += 2;
        pdata[-1] = len;
        pdata[-2] = len >> 8;
        pdata[-3] = 0;
        pdata[-4] = 1;
        pdata -= 4; len += 4;

        pdata[-1] = len;
        pdata[-2] = len >> 8;
        pdata -= 2; len += 2;
        pdata[-1] = 0x1;
        pdata[-2] = 0x3; // tls version
        pdata -= 2; len += 2;
        pdata[-1] = 0x16; // tls handshake
        pdata -= 1; len += 1;

        local->handshake_status = 1;
    } else if (datalength == 0) {
        datalength = local->send_buffer_size + 43;
        out_buffer = (char*)malloc(datalength);
        char *pdata = out_buffer;
        memcpy(pdata, "\x14\x03\x03\x00\x01\x01", 6);
        pdata += 6;
        memcpy(pdata, "\x16\x03\x03\x00\x20", 5);
        pdata += 5;
        rand_bytes((uint8_t*)pdata, 22);
        pdata += 22;

        uint8_t *key = (uint8_t*)malloc(self->server.key_len + 32);
        char hash[ONETIMEAUTH_BYTES * 2];
        memcpy(key, self->server.key, self->server.key_len);
        memcpy(key + self->server.key_len, global->local_client_id, 32);
        ss_sha1_hmac_with_key(hash, out_buffer, pdata - out_buffer, key, self->server.key_len + 32);
        free(key);
        memcpy(pdata, hash, OBFS_HMAC_SHA1_LEN);

        pdata += OBFS_HMAC_SHA1_LEN;
        memcpy(pdata, local->send_buffer, local->send_buffer_size);
        free(local->send_buffer);
        local->send_buffer = NULL;

        local->handshake_status = 8;
    } else {
        return 0;
    }
    if (*capacity < datalength) {
        *pencryptdata = (char*)realloc(*pencryptdata, *capacity = datalength * 2);
        encryptdata = *pencryptdata;
    }
    memmove(encryptdata, out_buffer, datalength);
    free(out_buffer);
    return datalength;
}

int tls12_ticket_auth_server_encode(obfs *self, char **pencryptdata, int datalength, size_t* capacity) {
    char *encryptdata = *pencryptdata;
    tls12_ticket_auth_local_data *local = (tls12_ticket_auth_local_data*)self->l_data;
    tls12_ticket_auth_global_data *global = (tls12_ticket_auth_global_data*)self->server.g_data;
    char * out_buffer = NULL;

    if (local->handshake_status == 8) {
        if (datalength < 1024) {
            if (*capacity < datalength + 5) {
                *pencryptdata = (char*)realloc(*pencryptdata, *capacity = (datalength + 5) * 2);
                encryptdata = *pencryptdata;
            }
            memmove(encryptdata + 5, encryptdata, datalength);
            encryptdata[0] = 0x17;
            encryptdata[1] = 0x3;
            encryptdata[2] = 0x3;
            encryptdata[3] = datalength >> 8;
            encryptdata[4] = datalength;
            return datalength + 5;
        } else {
            out_buffer = (char*)malloc(datalength + 2048);
            int start = 0;
            int outlength = 0;
            int len;
            while (datalength - start > 2048) {
                len = xorshift128plus() % 4096 + 100;
                if (len > datalength - start)
                    len = datalength - start;
                tls12_ticket_auth_pack_data(encryptdata, datalength, start, len, out_buffer, outlength);
                outlength += len + 5;
                start += len;
            }
            if (datalength - start > 0) {
                len = datalength - start;
                tls12_ticket_auth_pack_data(encryptdata, datalength, start, len, out_buffer, outlength);
                outlength += len + 5;
            }
            if (*capacity < outlength) {
                *pencryptdata = (char*)realloc(*pencryptdata, *capacity = outlength * 2);
                encryptdata = *pencryptdata;
            }
            memcpy(encryptdata, out_buffer, outlength);
            free(out_buffer);
            return outlength;
        }
    }

    local->handshake_status = 3;

    out_buffer = (char*)malloc(43 + 86);
    int data_len = 0;
    char *p_data = out_buffer + 86;

    memcpy(p_data - 10, "\xc0\x2f\x00\x00\x05\xff\x01\x00\x01\x00", 10);
    p_data -= 10;data_len += 10;

    memcpy(p_data - 32, global->local_client_id, 32);
    p_data -= 32;data_len += 32;

    p_data[-1] = 0x20;
    p_data -= 1;data_len += 1;

    tls12_ticket_pack_auth_data(global, &self->server, p_data - 32);
    p_data -= 32;data_len += 32;

    p_data[-1] = 0x3;
    p_data[-2] = 0x3; // tls version
    p_data -= 2;data_len += 2;

    p_data[-1] = data_len;
    p_data[-2] = data_len >> 8;
    p_data[-3] = 0x00;
    p_data[-4] = 0x02;
    p_data -= 4; data_len += 4;

    p_data[-1] = data_len;
    p_data[-2] = data_len >> 8;
    p_data[-3] = 0x03;
    p_data[-4] = 0x03;
    p_data[-5] = 0x16;
    p_data -= 5; data_len += 5;

    memcpy(out_buffer, p_data, data_len);
    char *pdata = out_buffer + 86;

    memcpy(pdata, "\x14\x03\x03\x00\x01\x01", 6);
    pdata += 6;
    memcpy(pdata, "\x16\x03\x03\x00\x20", 5);
    pdata += 5;
    rand_bytes((uint8_t*)pdata, 22);
    pdata += 22;

    uint8_t *key = (uint8_t*)malloc(self->server.key_len + 32);
    char hash[ONETIMEAUTH_BYTES * 2];
    memcpy(key, self->server.key, self->server.key_len);
    memcpy(key + self->server.key_len, global->local_client_id, 32);
    ss_sha1_hmac_with_key(hash, out_buffer, 43 + 86, key, self->server.key_len + 32);
    free(key);
    memcpy(pdata, hash, OBFS_HMAC_SHA1_LEN);

    memmove(encryptdata, out_buffer, 43 + 86);
    free(out_buffer);
    return 43 + 86;
}

int tls12_ticket_auth_client_decode(obfs *self, char **pencryptdata, int datalength, size_t* capacity, int *needsendback) {
    char *encryptdata = *pencryptdata;
    tls12_ticket_auth_local_data *local = (tls12_ticket_auth_local_data*)self->l_data;
    tls12_ticket_auth_global_data *global = (tls12_ticket_auth_global_data*)self->server.g_data;

    *needsendback = 0;

    if (local->handshake_status == 8) {
        local->recv_buffer_size += datalength;
        local->recv_buffer = (char*)realloc(local->recv_buffer, local->recv_buffer_size);
        memcpy(local->recv_buffer + local->recv_buffer_size - datalength, encryptdata, datalength);
        datalength = 0;
        while (local->recv_buffer_size > 5) {
            if (local->recv_buffer[0] != 0x17)
                return -1;
            int size = ((int)(unsigned char)local->recv_buffer[3] << 8) + (unsigned char)local->recv_buffer[4];
            if (size + 5 > local->recv_buffer_size)
                break;
            if (*capacity < datalength + size) {
                *pencryptdata = (char*)realloc(*pencryptdata, *capacity = (datalength + size) * 2);
                encryptdata = *pencryptdata;
            }
            memcpy(encryptdata + datalength, local->recv_buffer + 5, size);
            datalength += size;
            local->recv_buffer_size -= 5 + size;
            memmove(local->recv_buffer, local->recv_buffer + 5 + size, local->recv_buffer_size);
        }
        return datalength;
    }
    if (datalength < 11 + 32 + 1 + 32) {
        return -1;
    }

    uint8_t *key = (uint8_t*)malloc(self->server.key_len + 32);
    char hash[ONETIMEAUTH_BYTES * 2];
    memcpy(key, self->server.key, self->server.key_len);
    memcpy(key + self->server.key_len, global->local_client_id, 32);
    ss_sha1_hmac_with_key(hash, encryptdata + 11, 22, key, self->server.key_len + 32);
    free(key);

    if (memcmp(encryptdata + 33, hash, OBFS_HMAC_SHA1_LEN)) {
        return -1;
    }

    *needsendback = 1;
    return 0;
}

int tls12_ticket_auth_server_decode(obfs *self, char **pencryptdata, int datalength, size_t* capacity, int *needsendback) {
    char *encryptdata = *pencryptdata;
    tls12_ticket_auth_local_data *local = (tls12_ticket_auth_local_data*)self->l_data;
    tls12_ticket_auth_global_data *global = (tls12_ticket_auth_global_data*)self->server.g_data;

    *needsendback = 0;

    if (local->handshake_status == 8) {
        if(datalength != 0)
        {
            local->recv_buffer = (char*)realloc(local->recv_buffer, local->recv_buffer_size + datalength);
            memmove(local->recv_buffer + local->recv_buffer_size, encryptdata, datalength);
            local->recv_buffer_size += datalength;
        }
        datalength = 0;

        while (local->recv_buffer_size > 5) {
            if (local->recv_buffer[0] != 0x17 || local->recv_buffer[1] != 0x03 || local->recv_buffer[2] != 0x03)
            {
                LOGE("server_decode data error, wrong tls version 3");
                return -1;
            }
            int size = ((int)(unsigned char)local->recv_buffer[3] << 8) + (unsigned char)local->recv_buffer[4];
            if (size + 5 > local->recv_buffer_size)
                break;
            if (*capacity < local->recv_buffer_size + size) {
                *pencryptdata = (char*)realloc(*pencryptdata, *capacity = (local->recv_buffer_size + size) * 2);
                encryptdata = *pencryptdata;
            }
            memcpy(encryptdata + datalength, local->recv_buffer + 5, size);
            datalength += size;
            local->recv_buffer_size -= 5 + size;
            memmove(local->recv_buffer, local->recv_buffer + 5 + size, local->recv_buffer_size);
        }
        return datalength;
    }

    if (local->handshake_status == 3) {

        char *verify = encryptdata;

        if(datalength < 43)
        {
            LOGE("server_decode data error, too short:%d", (int)datalength);
            return -1;
        }

        if(encryptdata[0] != 0x14 || encryptdata[1] != 0x03 || encryptdata[2] != 0x03 || encryptdata[3] != 0x00 || encryptdata[4] != 0x01 || encryptdata[5] != 0x01)
        {
            LOGE("server_decode data error, wrong tls version");
            return -1;
        }

        encryptdata += 6;

        if(encryptdata[0] != 0x16 || encryptdata[1] != 0x03 || encryptdata[2] != 0x03 || encryptdata[3] != 0x00 || encryptdata[4] != 0x20)
        {
            LOGE("server_decode data error, wrong tls version 2");
            return -1;
        }

        uint8_t *key = (uint8_t*)malloc(self->server.key_len + 32);
        char hash[ONETIMEAUTH_BYTES * 2];
        memcpy(key, self->server.key, self->server.key_len);
        memcpy(key + self->server.key_len, global->local_client_id, 32);
        ss_sha1_hmac_with_key(hash, verify, 33, key, self->server.key_len + 32);
        free(key);

        if (memcmp(verify + 33, hash, OBFS_HMAC_SHA1_LEN) != 0) {
            LOGE("server_decode data error, hash Mismatch %d",(int)memcmp(verify + 33, hash, OBFS_HMAC_SHA1_LEN));
            return -1;
        }

        local->recv_buffer_size = datalength - 43;
        local->recv_buffer = (char*)realloc(local->recv_buffer, local->recv_buffer_size);
        memmove(local->recv_buffer, encryptdata + 37, datalength - 43);

        local->handshake_status = 8;
        return tls12_ticket_auth_server_decode(self, pencryptdata, 0, capacity, needsendback);
    }

    local->handshake_status = 2;
    if(encryptdata[0] != 0x16 || encryptdata[1] != 0x03 || encryptdata[2] != 0x01)
    {
        return -1;
    }

    encryptdata += 3;

    {
        int size = ((int)(unsigned char)encryptdata[0] << 8) + (unsigned char)encryptdata[1];
        if(size != datalength - 5)
        {
            LOGE("tls_auth wrong tls head size");
            return -1;
        }
    }

    encryptdata += 2;

    if(encryptdata[0] != 0x01 || encryptdata[1] != 0x00)
    {
        LOGE("tls_auth not client hello message");
        return -1;
    }

    encryptdata += 2;

    {
        int size = ((int)(unsigned char)encryptdata[0] << 8) + (unsigned char)encryptdata[1];
        if(size != datalength - 9)
        {
            LOGE("tls_auth wrong message size");
            return -1;
        }
    }

    encryptdata += 2;

    if(encryptdata[0] != 0x03 || encryptdata[1] != 0x03)
    {
        LOGE("tls_auth wrong tls version");
        return -1;
    }

    encryptdata += 2;

    char *verifyid = encryptdata;

    encryptdata += 32;

    int sessionid_len = encryptdata[0];
    if(sessionid_len < 32)
    {
        LOGE("tls_auth wrong sessionid_len");
        return -1;
    }

    char *sessionid = encryptdata + 1;
    memcpy(global->local_client_id , sessionid, sessionid_len);

    uint8_t *key = (uint8_t*)malloc(self->server.key_len + sessionid_len);
    char hash[ONETIMEAUTH_BYTES * 2];
    memcpy(key, self->server.key, self->server.key_len);
    memcpy(key + self->server.key_len, global->local_client_id, sessionid_len);
    ss_sha1_hmac_with_key(hash, verifyid, 22, key, self->server.key_len + sessionid_len);
    free(key);

    encryptdata += (sessionid_len + 1);

    long utc_time = ((int)(unsigned char)verifyid[0] << 24) + ((int)(unsigned char)verifyid[1] << 16) + ((int)(unsigned char)verifyid[2] << 8) + (unsigned char)verifyid[3];
    time_t t = time(NULL);


    if (self->server.param && strlen(self->server.param) == 0)
    {
        self->server.param = NULL;
    }

    int max_time_dif = 0;
    int time_dif = utc_time - t;
    if(self->server.param)
    {
        max_time_dif = atoi(self->server.param);
    }

    if(max_time_dif > 0 && (time_dif < -max_time_dif || time_dif > max_time_dif || utc_time - global->startup_time < -max_time_dif / 2))
    {
        LOGE("tls_auth wrong time");
        return -1;
    }

    if (memcmp(verifyid + 22, hash, OBFS_HMAC_SHA1_LEN)) {
        LOGE("tls_auth wrong sha1");
        return -1;
    }

    int search_result = global->client_data->have_same_cmp(global->client_data, verifyid);
    if(search_result != 0)
    {
        LOGE("replay attack detect!");
        return -1;
    }

    global->client_data->add_back(global->client_data, verifyid);

    encryptdata += 48;

    *needsendback = 1;

    return 0;
}
