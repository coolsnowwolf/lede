
#include "auth.h"

static int auth_simple_pack_unit_size = 2000;
typedef int (*hmac_with_key_func)(char *auth, char *msg, int msg_len, uint8_t *auth_key, int key_len);
typedef int (*hash_func)(char *auth, char *msg, int msg_len);

typedef struct auth_simple_global_data {
    uint8_t local_client_id[8];
    uint32_t connection_id;
}auth_simple_global_data;

typedef struct auth_simple_local_data {
    int has_sent_header;
    char * recv_buffer;
    int recv_buffer_size;
    uint32_t recv_id;
    uint32_t pack_id;
    char * salt;
    uint8_t * user_key;
    char uid[4];
    int user_key_len;
    hmac_with_key_func hmac;
    hash_func hash;
    int hash_len;
}auth_simple_local_data;

void auth_simple_local_data_init(auth_simple_local_data* local) {
    local->has_sent_header = 0;
    local->recv_buffer = (char*)malloc(16384);
    local->recv_buffer_size = 0;
    local->recv_id = 1;
    local->pack_id = 1;
    local->salt = "";
    local->user_key = 0;
    local->user_key_len = 0;
    local->hmac = 0;
    local->hash = 0;
    local->hash_len = 0;
    local->salt = "";
}

void * auth_simple_init_data() {
    auth_simple_global_data *global = (auth_simple_global_data*)malloc(sizeof(auth_simple_global_data));
    rand_bytes(global->local_client_id, 8);
    rand_bytes((uint8_t*)&global->connection_id, 4);
    global->connection_id &= 0xFFFFFF;
    return global;
}

obfs * auth_simple_new_obfs() {
    obfs * self = new_obfs();
    self->l_data = malloc(sizeof(auth_simple_local_data));
    auth_simple_local_data_init((auth_simple_local_data*)self->l_data);
    return self;
}

obfs * auth_aes128_md5_new_obfs() {
    obfs * self = new_obfs();
    self->l_data = malloc(sizeof(auth_simple_local_data));
    auth_simple_local_data_init((auth_simple_local_data*)self->l_data);
    ((auth_simple_local_data*)self->l_data)->hmac = ss_md5_hmac_with_key;
    ((auth_simple_local_data*)self->l_data)->hash = ss_md5_hash_func;
    ((auth_simple_local_data*)self->l_data)->hash_len = 16;
    ((auth_simple_local_data*)self->l_data)->salt = "auth_aes128_md5";
    return self;
}

obfs * auth_aes128_sha1_new_obfs() {
    obfs * self = new_obfs();
    self->l_data = malloc(sizeof(auth_simple_local_data));
    auth_simple_local_data_init((auth_simple_local_data*)self->l_data);
    ((auth_simple_local_data*)self->l_data)->hmac = ss_sha1_hmac_with_key;
    ((auth_simple_local_data*)self->l_data)->hash = ss_sha1_hash_func;
    ((auth_simple_local_data*)self->l_data)->hash_len = 20;
    ((auth_simple_local_data*)self->l_data)->salt = "auth_aes128_sha1";
    return self;
}

void auth_simple_dispose(obfs *self) {
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;
    if (local->recv_buffer != NULL) {
        free(local->recv_buffer);
        local->recv_buffer = NULL;
    }
    if (local->user_key != NULL) {
        free(local->user_key);
        local->user_key = NULL;
    }
    free(local);
    self->l_data = NULL;
    dispose_obfs(self);
}

int auth_simple_pack_data(char *data, int datalength, char *outdata) {
    unsigned char rand_len = (xorshift128plus() & 0xF) + 1;
    int out_size = rand_len + datalength + 6;
    outdata[0] = out_size >> 8;
    outdata[1] = out_size;
    outdata[2] = rand_len;
    memmove(outdata + rand_len + 2, data, datalength);
    fillcrc32((unsigned char *)outdata, out_size);
    return out_size;
}

void memintcopy_lt(void *mem, uint32_t val) {
    ((uint8_t *)mem)[0] = val;
    ((uint8_t *)mem)[1] = val >> 8;
    ((uint8_t *)mem)[2] = val >> 16;
    ((uint8_t *)mem)[3] = val >> 24;
}

int auth_simple_pack_auth_data(auth_simple_global_data *global, char *data, int datalength, char *outdata) {
    unsigned char rand_len = (xorshift128plus() & 0xF) + 1;
    int out_size = rand_len + datalength + 6 + 12;
    outdata[0] = out_size >> 8;
    outdata[1] = out_size;
    outdata[2] = rand_len;
    ++global->connection_id;
    if (global->connection_id > 0xFF000000) {
        rand_bytes(global->local_client_id, 8);
        rand_bytes((uint8_t*)&global->connection_id, 4);
        global->connection_id &= 0xFFFFFF;
    }
    time_t t = time(NULL);
    memintcopy_lt(outdata + rand_len + 2, t);
    memmove(outdata + rand_len + 2 + 4, global->local_client_id, 4);
    memintcopy_lt(outdata + rand_len + 2 + 8, global->connection_id);
    memmove(outdata + rand_len + 2 + 12, data, datalength);
    fillcrc32((unsigned char *)outdata, out_size);
    return out_size;
}

int auth_simple_client_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity) {
    char *plaindata = *pplaindata;
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;
    char * out_buffer = (char*)malloc(datalength * 2 + 64);
    char * buffer = out_buffer;
    char * data = plaindata;
    int len = datalength;
    int pack_len;
    if (len > 0 && local->has_sent_header == 0) {
        int head_size = get_head_size(plaindata, datalength, 30);
        if (head_size > datalength)
            head_size = datalength;
        pack_len = auth_simple_pack_auth_data((auth_simple_global_data *)self->server.g_data, data, head_size, buffer);
        buffer += pack_len;
        data += head_size;
        len -= head_size;
        local->has_sent_header = 1;
    }
    while ( len > auth_simple_pack_unit_size ) {
        pack_len = auth_simple_pack_data(data, auth_simple_pack_unit_size, buffer);
        buffer += pack_len;
        data += auth_simple_pack_unit_size;
        len -= auth_simple_pack_unit_size;
    }
    if (len > 0) {
        pack_len = auth_simple_pack_data(data, len, buffer);
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

int auth_simple_client_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity) {
    char *plaindata = *pplaindata;
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;
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


int auth_sha1_pack_data(char *data, int datalength, char *outdata) {
    unsigned char rand_len = (xorshift128plus() & 0xF) + 1;
    int out_size = rand_len + datalength + 6;
    outdata[0] = out_size >> 8;
    outdata[1] = out_size;
    outdata[2] = rand_len;
    memmove(outdata + rand_len + 2, data, datalength);
    filladler32((unsigned char *)outdata, out_size);
    return out_size;
}

int auth_sha1_pack_auth_data(auth_simple_global_data *global, server_info *server, char *data, int datalength, char *outdata) {
    unsigned char rand_len = (xorshift128plus() & 0x7F) + 1;
    int data_offset = rand_len + 4 + 2;
    int out_size = data_offset + datalength + 12 + OBFS_HMAC_SHA1_LEN;
    fillcrc32to((unsigned char *)server->key, server->key_len, (unsigned char *)outdata);
    outdata[4] = out_size >> 8;
    outdata[5] = out_size;
    outdata[6] = rand_len;
    ++global->connection_id;
    if (global->connection_id > 0xFF000000) {
        rand_bytes(global->local_client_id, 8);
        rand_bytes((uint8_t*)&global->connection_id, 4);
        global->connection_id &= 0xFFFFFF;
    }
    time_t t = time(NULL);
    memintcopy_lt(outdata + data_offset, t);
    memmove(outdata + data_offset + 4, global->local_client_id, 4);
    memintcopy_lt(outdata + data_offset + 8, global->connection_id);
    memmove(outdata + data_offset + 12, data, datalength);
    char hash[ONETIMEAUTH_BYTES * 2];
    ss_sha1_hmac(hash, outdata, out_size - OBFS_HMAC_SHA1_LEN, server->iv);
    memcpy(outdata + out_size - OBFS_HMAC_SHA1_LEN, hash, OBFS_HMAC_SHA1_LEN);
    return out_size;
}

int auth_sha1_client_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity) {
    char *plaindata = *pplaindata;
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;
    char * out_buffer = (char*)malloc(datalength * 2 + 256);
    char * buffer = out_buffer;
    char * data = plaindata;
    int len = datalength;
    int pack_len;
    if (len > 0 && local->has_sent_header == 0) {
        int head_size = get_head_size(plaindata, datalength, 30);
        if (head_size > datalength)
            head_size = datalength;
        pack_len = auth_sha1_pack_auth_data((auth_simple_global_data *)self->server.g_data, &self->server, data, head_size, buffer);
        buffer += pack_len;
        data += head_size;
        len -= head_size;
        local->has_sent_header = 1;
    }
    while ( len > auth_simple_pack_unit_size ) {
        pack_len = auth_sha1_pack_data(data, auth_simple_pack_unit_size, buffer);
        buffer += pack_len;
        data += auth_simple_pack_unit_size;
        len -= auth_simple_pack_unit_size;
    }
    if (len > 0) {
        pack_len = auth_sha1_pack_data(data, len, buffer);
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

int auth_sha1_client_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity) {
    char *plaindata = *pplaindata;
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;
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

        if (checkadler32((unsigned char*)recv_buffer, length) == 0) {
            free(out_buffer);
            local->recv_buffer_size = 0;
            return -1;
        }
        int pos = recv_buffer[2] + 2;
        int data_size = length - pos - 4;
        memmove(buffer, recv_buffer + pos, data_size);
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

int auth_sha1_v2_pack_data(char *data, int datalength, char *outdata) {
    unsigned int rand_len = (datalength > 1300 ? 0 : datalength > 400 ? (xorshift128plus() & 0x7F) : (xorshift128plus() & 0x3FF)) + 1;
    int out_size = rand_len + datalength + 6;
    outdata[0] = out_size >> 8;
    outdata[1] = out_size;
    if (rand_len < 128)
    {
        outdata[2] = rand_len;
    }
    else
    {
        outdata[2] = 0xFF;
        outdata[3] = rand_len >> 8;
        outdata[4] = rand_len;
    }
    memmove(outdata + rand_len + 2, data, datalength);
    filladler32((unsigned char *)outdata, out_size);
    return out_size;
}

int auth_sha1_v2_pack_auth_data(auth_simple_global_data *global, server_info *server, char *data, int datalength, char *outdata) {
    unsigned int rand_len = (datalength > 1300 ? 0 : datalength > 400 ? (xorshift128plus() & 0x7F) : (xorshift128plus() & 0x3FF)) + 1;
    int data_offset = rand_len + 4 + 2;
    int out_size = data_offset + datalength + 12 + OBFS_HMAC_SHA1_LEN;
    const char* salt = "auth_sha1_v2";
    int salt_len = strlen(salt);
    unsigned char *crc_salt = (unsigned char*)malloc(salt_len + server->key_len);
    memcpy(crc_salt, salt, salt_len);
    memcpy(crc_salt + salt_len, server->key, server->key_len);
    fillcrc32to(crc_salt, salt_len + server->key_len, (unsigned char *)outdata);
    free(crc_salt);
    outdata[4] = out_size >> 8;
    outdata[5] = out_size;
    if (rand_len < 128)
    {
        outdata[6] = rand_len;
    }
    else
    {
        outdata[6] = 0xFF;
        outdata[7] = rand_len >> 8;
        outdata[8] = rand_len;
    }
    ++global->connection_id;
    if (global->connection_id > 0xFF000000) {
        rand_bytes(global->local_client_id, 8);
        rand_bytes((uint8_t*)&global->connection_id, 4);
        global->connection_id &= 0xFFFFFF;
    }
    memmove(outdata + data_offset, global->local_client_id, 8);
    memintcopy_lt(outdata + data_offset + 8, global->connection_id);
    memmove(outdata + data_offset + 12, data, datalength);
    char hash[ONETIMEAUTH_BYTES * 2];
    ss_sha1_hmac(hash, outdata, out_size - OBFS_HMAC_SHA1_LEN, server->iv);
    memcpy(outdata + out_size - OBFS_HMAC_SHA1_LEN, hash, OBFS_HMAC_SHA1_LEN);
    return out_size;
}

int auth_sha1_v2_client_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity) {
    char *plaindata = *pplaindata;
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;
    char * out_buffer = (char*)malloc(datalength * 2 + 4096);
    char * buffer = out_buffer;
    char * data = plaindata;
    int len = datalength;
    int pack_len;
    if (len > 0 && local->has_sent_header == 0) {
        int head_size = get_head_size(plaindata, datalength, 30);
        if (head_size > datalength)
            head_size = datalength;
        pack_len = auth_sha1_v2_pack_auth_data((auth_simple_global_data *)self->server.g_data, &self->server, data, head_size, buffer);
        buffer += pack_len;
        data += head_size;
        len -= head_size;
        local->has_sent_header = 1;
    }
    while ( len > auth_simple_pack_unit_size ) {
        pack_len = auth_sha1_v2_pack_data(data, auth_simple_pack_unit_size, buffer);
        buffer += pack_len;
        data += auth_simple_pack_unit_size;
        len -= auth_simple_pack_unit_size;
    }
    if (len > 0) {
        pack_len = auth_sha1_v2_pack_data(data, len, buffer);
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

int auth_sha1_v2_client_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity) {
    char *plaindata = *pplaindata;
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;
    uint8_t * recv_buffer = (uint8_t *)local->recv_buffer;
    if (local->recv_buffer_size + datalength > 16384)
        return -1;
    memmove(recv_buffer + local->recv_buffer_size, plaindata, datalength);
    local->recv_buffer_size += datalength;

    char * out_buffer = (char*)malloc(local->recv_buffer_size);
    char * buffer = out_buffer;
    char error = 0;
    while (local->recv_buffer_size > 2) {
        int length = ((int)recv_buffer[0] << 8) | recv_buffer[1];
        if (length >= 8192 || length < 7) {
            local->recv_buffer_size = 0;
            error = 1;
            break;
        }
        if (length > local->recv_buffer_size)
            break;

        if (checkadler32((unsigned char*)recv_buffer, length) == 0) {
            local->recv_buffer_size = 0;
            error = 1;
            break;
        }
        int pos = recv_buffer[2];
        if (pos < 255)
        {
            pos += 2;
        }
        else
        {
            pos = ((recv_buffer[3] << 8) | recv_buffer[4]) + 2;
        }
        int data_size = length - pos - 4;
        memmove(buffer, recv_buffer + pos, data_size);
        buffer += data_size;
        memmove(recv_buffer, recv_buffer + length, local->recv_buffer_size -= length);
    }
    int len;
    if (error == 0) {
        len = buffer - out_buffer;
        if (*capacity < len) {
            *pplaindata = (char*)realloc(*pplaindata, *capacity = len * 2);
            plaindata = *pplaindata;
        }
        memmove(plaindata, out_buffer, len);
    } else {
        len = -1;
    }
    free(out_buffer);
    return len;
}

int auth_sha1_v4_pack_data(char *data, int datalength, char *outdata) {
    unsigned int rand_len = (datalength > 1300 ? 0 : datalength > 400 ? (xorshift128plus() & 0x7F) : (xorshift128plus() & 0x3FF)) + 1;
    int out_size = rand_len + datalength + 8;
    outdata[0] = out_size >> 8;
    outdata[1] = out_size;
    uint32_t crc_val = crc32((unsigned char*)outdata, 2);
    outdata[2] = crc_val;
    outdata[3] = crc_val >> 8;
    if (rand_len < 128)
    {
        outdata[4] = rand_len;
    }
    else
    {
        outdata[4] = 0xFF;
        outdata[5] = rand_len >> 8;
        outdata[6] = rand_len;
    }
    memmove(outdata + rand_len + 4, data, datalength);
    filladler32((unsigned char *)outdata, out_size);
    return out_size;
}

int auth_sha1_v4_pack_auth_data(auth_simple_global_data *global, server_info *server, char *data, int datalength, char *outdata) {
    unsigned int rand_len = (datalength > 1300 ? 0 : datalength > 400 ? (xorshift128plus() & 0x7F) : (xorshift128plus() & 0x3FF)) + 1;
    int data_offset = rand_len + 4 + 2;
    int out_size = data_offset + datalength + 12 + OBFS_HMAC_SHA1_LEN;
    const char* salt = "auth_sha1_v4";
    int salt_len = strlen(salt);
    unsigned char *crc_salt = (unsigned char*)malloc(salt_len + server->key_len + 2);
    crc_salt[0] = outdata[0] = out_size >> 8;
    crc_salt[1] = outdata[1] = out_size;

    memcpy(crc_salt + 2, salt, salt_len);
    memcpy(crc_salt + salt_len + 2, server->key, server->key_len);
    fillcrc32to(crc_salt, salt_len + server->key_len + 2, (unsigned char *)outdata + 2);
    free(crc_salt);
    if (rand_len < 128)
    {
        outdata[6] = rand_len;
    }
    else
    {
        outdata[6] = 0xFF;
        outdata[7] = rand_len >> 8;
        outdata[8] = rand_len;
    }
    ++global->connection_id;
    if (global->connection_id > 0xFF000000) {
        rand_bytes(global->local_client_id, 8);
        rand_bytes((uint8_t*)&global->connection_id, 4);
        global->connection_id &= 0xFFFFFF;
    }
    time_t t = time(NULL);
    memintcopy_lt(outdata + data_offset, t);
    memmove(outdata + data_offset + 4, global->local_client_id, 4);
    memintcopy_lt(outdata + data_offset + 8, global->connection_id);
    memmove(outdata + data_offset + 12, data, datalength);
    char hash[ONETIMEAUTH_BYTES * 2];
    ss_sha1_hmac(hash, outdata, out_size - OBFS_HMAC_SHA1_LEN, server->iv);
    memcpy(outdata + out_size - OBFS_HMAC_SHA1_LEN, hash, OBFS_HMAC_SHA1_LEN);
    return out_size;
}

int auth_sha1_v4_client_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity) {
    char *plaindata = *pplaindata;
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;
    char * out_buffer = (char*)malloc(datalength * 2 + 4096);
    char * buffer = out_buffer;
    char * data = plaindata;
    int len = datalength;
    int pack_len;
    if (len > 0 && local->has_sent_header == 0) {
        int head_size = get_head_size(plaindata, datalength, 30);
        if (head_size > datalength)
            head_size = datalength;
        pack_len = auth_sha1_v4_pack_auth_data((auth_simple_global_data *)self->server.g_data, &self->server, data, head_size, buffer);
        buffer += pack_len;
        data += head_size;
        len -= head_size;
        local->has_sent_header = 1;
    }
    while ( len > auth_simple_pack_unit_size ) {
        pack_len = auth_sha1_v4_pack_data(data, auth_simple_pack_unit_size, buffer);
        buffer += pack_len;
        data += auth_simple_pack_unit_size;
        len -= auth_simple_pack_unit_size;
    }
    if (len > 0) {
        pack_len = auth_sha1_v4_pack_data(data, len, buffer);
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

int auth_sha1_v4_client_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity) {
    char *plaindata = *pplaindata;
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;
    uint8_t * recv_buffer = (uint8_t *)local->recv_buffer;
    if (local->recv_buffer_size + datalength > 16384)
        return -1;
    memmove(recv_buffer + local->recv_buffer_size, plaindata, datalength);
    local->recv_buffer_size += datalength;

    char * out_buffer = (char*)malloc(local->recv_buffer_size);
    char * buffer = out_buffer;
    char error = 0;
    while (local->recv_buffer_size > 4) {
        uint32_t crc_val = crc32((unsigned char*)recv_buffer, 2);
        if ((((uint32_t)recv_buffer[3] << 8) | recv_buffer[2]) != (crc_val & 0xffff)) {
            local->recv_buffer_size = 0;
            error = 1;
            break;
        }
        int length = ((int)recv_buffer[0] << 8) | recv_buffer[1];
        if (length >= 8192 || length < 7) {
            local->recv_buffer_size = 0;
            error = 1;
            break;
        }
        if (length > local->recv_buffer_size)
            break;

        if (checkadler32((unsigned char*)recv_buffer, length) == 0) {
            local->recv_buffer_size = 0;
            error = 1;
            break;
        }
        int pos = recv_buffer[4];
        if (pos < 255)
        {
            pos += 4;
        }
        else
        {
            pos = (((int)recv_buffer[5] << 8) | recv_buffer[6]) + 4;
        }
        int data_size = length - pos - 4;
        memmove(buffer, recv_buffer + pos, data_size);
        buffer += data_size;
        memmove(recv_buffer, recv_buffer + length, local->recv_buffer_size -= length);
    }
    int len;
    if (error == 0) {
        len = buffer - out_buffer;
        if (*capacity < len) {
            *pplaindata = (char*)realloc(*pplaindata, *capacity = len * 2);
            plaindata = *pplaindata;
        }
        memmove(plaindata, out_buffer, len);
    } else {
        len = -1;
    }
    free(out_buffer);
    return len;
}


int auth_aes128_sha1_pack_data(char *data, int datalength, char *outdata, auth_simple_local_data *local, server_info *server) {
    unsigned int rand_len = (datalength > 1200 ? 0 : local->pack_id > 4 ? (xorshift128plus() & 0x20) : datalength > 900 ? (xorshift128plus() & 0x80) : (xorshift128plus() & 0x200)) + 1;
    int out_size = rand_len + datalength + 8;
    memcpy(outdata + rand_len + 4, data, datalength);
    outdata[0] = out_size;
    outdata[1] = out_size >> 8;
    uint8_t key_len = local->user_key_len + 4;
    uint8_t *key = (uint8_t*)malloc(key_len);
    memcpy(key, local->user_key, local->user_key_len);
    memintcopy_lt(key + key_len - 4, local->pack_id);

    {
        uint8_t rnd_data[rand_len];
        rand_bytes(rnd_data, rand_len);
        memcpy(outdata + 4, rnd_data, rand_len);
    }

    {
        char hash[20];
        local->hmac(hash, outdata, 2, key, key_len);
        memcpy(outdata + 2, hash, 2);
    }

    if (rand_len < 128)
    {
        outdata[4] = rand_len;
    }
    else
    {
        outdata[4] = 0xFF;
        outdata[5] = rand_len;
        outdata[6] = rand_len >> 8;
    }
    ++local->pack_id;

    {
        char hash[20];
        local->hmac(hash, outdata, out_size - 4, key, key_len);
        memcpy(outdata + out_size - 4, hash, 4);
    }
    free(key);

    return out_size;
}

int auth_aes128_sha1_pack_auth_data(auth_simple_global_data *global, server_info *server, auth_simple_local_data *local, char *data, int datalength, char *outdata) {
    unsigned int rand_len = (datalength > 400 ? (xorshift128plus() & 0x200) : (xorshift128plus() & 0x400));
    int data_offset = rand_len + 16 + 4 + 4 + 7;
    int out_size = data_offset + datalength + 4;

    char encrypt[24];
    char encrypt_data[16];

    uint8_t *key = (uint8_t*)malloc(server->iv_len + server->key_len);
    uint8_t key_len = server->iv_len + server->key_len;
    memcpy(key, server->iv, server->iv_len);
    memcpy(key + server->iv_len, server->key, server->key_len);

    {
        uint8_t rnd_data[rand_len];
        rand_bytes(rnd_data, rand_len);
        memcpy(outdata + data_offset - rand_len, rnd_data, rand_len);
    }

    ++global->connection_id;
    if (global->connection_id > 0xFF000000) {
        rand_bytes(global->local_client_id, 8);
        rand_bytes((uint8_t*)&global->connection_id, 4);
        global->connection_id &= 0xFFFFFF;
    }
    time_t t = time(NULL);
    memintcopy_lt(encrypt, t);
    memcpy(encrypt + 4, global->local_client_id, 4);
    memintcopy_lt(encrypt + 8, global->connection_id);
    encrypt[12] = out_size;
    encrypt[13] = out_size >> 8;
    encrypt[14] = rand_len;
    encrypt[15] = rand_len >> 8;

    {

        if (local->user_key == NULL) {
            if(server->param != NULL && server->param[0] != 0) {
                char *param = server->param;
                char *delim = strchr(param, ':');
                if(delim != NULL) {
                    char uid_str[16] = {};
                    strncpy(uid_str, param, delim - param);
                    char key_str[128];
                    strcpy(key_str, delim + 1);
                    long uid_long = strtol(uid_str, NULL, 10);
                    memintcopy_lt(local->uid, uid_long);

                    char hash[21] = {0};
                    local->hash(hash, key_str, strlen(key_str));

                    local->user_key_len = local->hash_len;
                    local->user_key = (uint8_t*)malloc(local->user_key_len);
                    memcpy(local->user_key, hash, local->hash_len);
                }
            }
            if (local->user_key == NULL) {
                rand_bytes((uint8_t *)local->uid, 4);

                local->user_key_len = server->key_len;
                local->user_key = (uint8_t*)malloc(local->user_key_len);
                memcpy(local->user_key, server->key, local->user_key_len);
            }
        }

        char encrypt_key_base64[256] = {0};
        unsigned char encrypt_key[local->user_key_len];
        memcpy(encrypt_key, local->user_key, local->user_key_len);
        base64_encode(encrypt_key, local->user_key_len, encrypt_key_base64);

        int base64_len;
        base64_len = (local->user_key_len + 2) / 3 * 4;
        memcpy(encrypt_key_base64 + base64_len, local->salt, strlen(local->salt));

        char enc_key[16];
        int enc_key_len = base64_len + strlen(local->salt);
        bytes_to_key_with_size(encrypt_key_base64, enc_key_len, (uint8_t*)enc_key, 16);
        ss_aes_128_cbc(encrypt, encrypt_data, enc_key);
        memcpy(encrypt + 4, encrypt_data, 16);
        memcpy(encrypt, local->uid, 4);
    }

    {
        char hash[20];
        local->hmac(hash, encrypt, 20, key, key_len);
        memcpy(encrypt + 20, hash, 4);
    }

    {
        uint8_t rnd[1];
        rand_bytes(rnd, 1);
        memcpy(outdata, rnd, 1);
        char hash[20];
        local->hmac(hash, (char *)rnd, 1, key, key_len);
        memcpy(outdata + 1, hash, 6);
    }

    memcpy(outdata + 7, encrypt, 24);
    memcpy(outdata + data_offset, data, datalength);

    {
        char hash[20];
        local->hmac(hash, outdata, out_size - 4, local->user_key, local->user_key_len);
        memmove(outdata + out_size - 4, hash, 4);
    }
    free(key);

    return out_size;
}

int auth_aes128_sha1_client_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity) {
    char *plaindata = *pplaindata;
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;
    char * out_buffer = (char*)malloc(datalength * 2 + 4096);
    char * buffer = out_buffer;
    char * data = plaindata;
    int len = datalength;
    int pack_len;
    if (len > 0 && local->has_sent_header == 0) {
        int head_size = 1200;
        if (head_size > datalength)
            head_size = datalength;
        pack_len = auth_aes128_sha1_pack_auth_data((auth_simple_global_data *)self->server.g_data, &self->server, local, data, head_size, buffer);
        buffer += pack_len;
        data += head_size;
        len -= head_size;
        local->has_sent_header = 1;
    }
    while ( len > auth_simple_pack_unit_size ) {
        pack_len = auth_aes128_sha1_pack_data(data, auth_simple_pack_unit_size, buffer, local, &self->server);
        buffer += pack_len;
        data += auth_simple_pack_unit_size;
        len -= auth_simple_pack_unit_size;
    }
    if (len > 0) {
        pack_len = auth_aes128_sha1_pack_data(data, len, buffer, local, &self->server);
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

int auth_aes128_sha1_client_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity) {
    char *plaindata = *pplaindata;
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;
    //server_info *server = (server_info*)&self->server;
    uint8_t * recv_buffer = (uint8_t *)local->recv_buffer;
    if (local->recv_buffer_size + datalength > 16384)
        return -1;
    memmove(recv_buffer + local->recv_buffer_size, plaindata, datalength);
    local->recv_buffer_size += datalength;

    int key_len = local->user_key_len + 4;
    uint8_t *key = (uint8_t*)malloc(key_len);
    memcpy(key, local->user_key, local->user_key_len);

    char * out_buffer = (char*)malloc(local->recv_buffer_size);
    char * buffer = out_buffer;
    char error = 0;
    while (local->recv_buffer_size > 4) {
        memintcopy_lt(key + key_len - 4, local->recv_id);

        {
            char hash[20];
            local->hmac(hash, (char*)recv_buffer, 2, key, key_len);

            if (memcmp(hash, recv_buffer + 2, 2)) {
                local->recv_buffer_size = 0;
                error = 1;
                break;
            }
        }

        int length = ((int)recv_buffer[1] << 8) + recv_buffer[0];
        if (length >= 8192 || length < 8) {
            local->recv_buffer_size = 0;
            error = 1;
            break;
        }
        if (length > local->recv_buffer_size)
            break;

        {
            char hash[20];
            local->hmac(hash, (char *)recv_buffer, length - 4, key, key_len);
            if (memcmp(hash, recv_buffer + length - 4, 4))
            {
                local->recv_buffer_size = 0;
                error = 1;
                break;
            }
        }

        ++local->recv_id;
        int pos = recv_buffer[4];
        if (pos < 255)
        {
            pos += 4;
        }
        else
        {
            pos = (((int)recv_buffer[6] << 8) | recv_buffer[5]) + 4;
        }
        int data_size = length - pos - 4;
        memmove(buffer, recv_buffer + pos, data_size);
        buffer += data_size;
        memmove(recv_buffer, recv_buffer + length, local->recv_buffer_size -= length);
    }
    int len;
    if (error == 0) {
        len = buffer - out_buffer;
        if (*capacity < len) {
            *pplaindata = (char*)realloc(*pplaindata, *capacity = len * 2);
            plaindata = *pplaindata;
        }
        memmove(plaindata, out_buffer, len);
    } else {
        len = -1;
    }
    free(out_buffer);
    free(key);
    return len;
}

int auth_aes128_sha1_client_udp_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity) {
    char *plaindata = *pplaindata;
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;
    char * out_buffer = (char*)malloc(datalength + 8);

    if (local->user_key == NULL) {
        if(self->server.param != NULL && self->server.param[0] != 0) {
            char *param = self->server.param;
            char *delim = strchr(param, ':');
            if(delim != NULL) {
                char uid_str[16] = {};
                strncpy(uid_str, param, delim - param);
                char key_str[128];
                strcpy(key_str, delim + 1);
                long uid_long = strtol(uid_str, NULL, 10);
                memintcopy_lt(local->uid, uid_long);

                char hash[21] = {0};
                local->hash(hash, key_str, strlen(key_str));

                local->user_key_len = local->hash_len;
                local->user_key = (uint8_t*)malloc(local->user_key_len);
                memcpy(local->user_key, hash, local->hash_len);
            }
        }
        if (local->user_key == NULL) {
            rand_bytes((uint8_t *)local->uid, 4);

            local->user_key_len = self->server.key_len;
            local->user_key = (uint8_t*)malloc(local->user_key_len);
            memcpy(local->user_key, self->server.key, local->user_key_len);
        }
    }

    int outlength = datalength + 8;
    memmove(out_buffer, plaindata, datalength);
    memmove(out_buffer + datalength, local->uid, 4);

    {
        char hash[20];
        local->hmac(hash, out_buffer, outlength - 4, local->user_key, local->user_key_len);
        memmove(out_buffer + outlength - 4, hash, 4);
    }

    if (*capacity < outlength) {
        *pplaindata = (char*)realloc(*pplaindata, *capacity = outlength * 2);
        plaindata = *pplaindata;
    }
    memmove(plaindata, out_buffer, outlength);

    free(out_buffer);
    return outlength;
}

int auth_aes128_sha1_client_udp_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity) {
    if (datalength <= 4)
        return 0;

    char *plaindata = *pplaindata;
    auth_simple_local_data *local = (auth_simple_local_data*)self->l_data;

    char hash[20];
    local->hmac(hash, plaindata, datalength - 4, self->server.key, self->server.key_len);

    if (memcmp(hash, plaindata + datalength - 4, 4))
    {
        return 0;
    }

    return datalength - 4;
}
