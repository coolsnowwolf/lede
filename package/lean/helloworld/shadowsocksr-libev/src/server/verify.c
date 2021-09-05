
#include "verify.h"

static int verify_simple_pack_unit_size = 2000;

typedef struct verify_simple_local_data {
    char * recv_buffer;
    int recv_buffer_size;
}verify_simple_local_data;

void verify_simple_local_data_init(verify_simple_local_data* local) {
    local->recv_buffer = (char*)malloc(16384);
    local->recv_buffer_size = 0;
}

obfs * verify_simple_new_obfs() {
    obfs * self = new_obfs();
    self->l_data = malloc(sizeof(verify_simple_local_data));
    verify_simple_local_data_init((verify_simple_local_data*)self->l_data);
    return self;
}

void verify_simple_dispose(obfs *self) {
    verify_simple_local_data *local = (verify_simple_local_data*)self->l_data;
    if (local->recv_buffer != NULL) {
        free(local->recv_buffer);
        local->recv_buffer = NULL;
    }
    free(local);
    self->l_data = NULL;
    dispose_obfs(self);
}

int verify_simple_pack_data(char *data, int datalength, char *outdata) {
    unsigned char rand_len = (xorshift128plus() & 0xF) + 1;
    int out_size = rand_len + datalength + 6;
    outdata[0] = out_size >> 8;
    outdata[1] = out_size;
    outdata[2] = rand_len;
    memmove(outdata + rand_len + 2, data, datalength);
    fillcrc32((unsigned char *)outdata, out_size);
    return out_size;
}

int verify_simple_client_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t *capacity) {
    char *plaindata = *pplaindata;
    //verify_simple_local_data *local = (verify_simple_local_data*)self->l_data;
    char * out_buffer = (char*)malloc(datalength * 2 + 32);
    char * buffer = out_buffer;
    char * data = plaindata;
    int len = datalength;
    int pack_len;
    while ( len > verify_simple_pack_unit_size ) {
        pack_len = verify_simple_pack_data(data, verify_simple_pack_unit_size, buffer);
        buffer += pack_len;
        data += verify_simple_pack_unit_size;
        len -= verify_simple_pack_unit_size;
    }
    if (len > 0) {
        pack_len = verify_simple_pack_data(data, len, buffer);
        buffer += pack_len;
    }
    len = buffer - out_buffer;
    if (*capacity < len) {
        *pplaindata = (char*)realloc(*pplaindata, *capacity = len * 2);
        plaindata = *pplaindata;
    }
    memmove(plaindata, out_buffer, len);
    free(out_buffer);
    return len;
}

int verify_simple_client_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t *capacity) {
    char *plaindata = *pplaindata;
    verify_simple_local_data *local = (verify_simple_local_data*)self->l_data;
    uint8_t * recv_buffer = (uint8_t *)local->recv_buffer;
    if (local->recv_buffer_size + datalength > 16384)
        return -1;
    memmove(recv_buffer + local->recv_buffer_size, plaindata, datalength);
    local->recv_buffer_size += datalength;

    char * out_buffer = (char*)malloc(local->recv_buffer_size);
    char * buffer = out_buffer;
    while (local->recv_buffer_size > 2) {
        int length = ((int)recv_buffer[0] << 8) | recv_buffer[1];
        if (length >= 8192 || length < 7) {
            free(out_buffer);
            local->recv_buffer_size = 0;
            return -1;
        }
        if (length > local->recv_buffer_size)
            break;

        int crc = crc32((unsigned char*)recv_buffer, length);
        if (crc != -1) {
            free(out_buffer);
            local->recv_buffer_size = 0;
            return -1;
        }
        int data_size = length - recv_buffer[2] - 6;
        memmove(buffer, recv_buffer + 2 + recv_buffer[2], data_size);
        buffer += data_size;
        memmove(recv_buffer, recv_buffer + length, local->recv_buffer_size -= length);
    }
    int len = buffer - out_buffer;
    if (*capacity < len) {
        *pplaindata = (char*)realloc(*pplaindata, *capacity = len * 2);
        plaindata = *pplaindata;
    }
    memmove(plaindata, out_buffer, len);
    free(out_buffer);
    return len;
}

int verify_simple_server_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t *capacity) {
    char *plaindata = *pplaindata;
    //verify_simple_local_data *local = (verify_simple_local_data*)self->l_data;
    char * out_buffer = (char*)malloc(datalength * 2 + 32);
    char * buffer = out_buffer;
    char * data = plaindata;
    int len = datalength;
    int pack_len;
    while ( len > verify_simple_pack_unit_size ) {
        pack_len = verify_simple_pack_data(data, verify_simple_pack_unit_size, buffer);
        buffer += pack_len;
        data += verify_simple_pack_unit_size;
        len -= verify_simple_pack_unit_size;
    }
    if (len > 0) {
        pack_len = verify_simple_pack_data(data, len, buffer);
        buffer += pack_len;
    }
    len = buffer - out_buffer;
    if (*capacity < len) {
        *pplaindata = (char*)realloc(*pplaindata, *capacity = len * 2);
        plaindata = *pplaindata;
    }
    memmove(plaindata, out_buffer, len);
    free(out_buffer);
    return len;
}

int verify_simple_server_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t *capacity) {
    char *plaindata = *pplaindata;
    verify_simple_local_data *local = (verify_simple_local_data*)self->l_data;
    uint8_t * recv_buffer = (uint8_t *)local->recv_buffer;
    if (local->recv_buffer_size + datalength > 16384)
    {
        LOGE("verify_simple: wrong buf length %d", local->recv_buffer_size + datalength);
        return -1;
    }
    memmove(recv_buffer + local->recv_buffer_size, plaindata, datalength);
    local->recv_buffer_size += datalength;

    char * out_buffer = (char*)malloc(local->recv_buffer_size);
    char * buffer = out_buffer;
    while (local->recv_buffer_size > 2) {
        int length = ((int)recv_buffer[0] << 8) | recv_buffer[1];
        if (length >= 8192 || length < 7) {
            free(out_buffer);
            local->recv_buffer_size = 0;
            LOGE("verify_simple: wrong length %d", length);
            return -1;
        }
        if (length > local->recv_buffer_size)
            break;

        int crc = crc32((unsigned char*)recv_buffer, length);
        if (crc != -1) {
            free(out_buffer);
            local->recv_buffer_size = 0;
            LOGE("verify_simple: wrong crc");
            return -1;
        }
        int data_size = length - recv_buffer[2] - 6;
        memmove(buffer, recv_buffer + 2 + recv_buffer[2], data_size);
        buffer += data_size;
        memmove(recv_buffer, recv_buffer + length, local->recv_buffer_size -= length);
    }
    int len = buffer - out_buffer;
    if (*capacity < len) {
        *pplaindata = (char*)realloc(*pplaindata, *capacity = len * 2);
        plaindata = *pplaindata;
    }
    memmove(plaindata, out_buffer, len);
    free(out_buffer);
    return len;
}
