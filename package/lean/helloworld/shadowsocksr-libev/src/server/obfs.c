#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "obfs.h"

int rand_bytes(uint8_t *output, int len);
#define OBFS_HMAC_SHA1_LEN 10

#include "obfsutil.c"
#include "crc32.c"
#include "base64.c"
#include "http_simple.c"
#include "tls1.2_ticket.c"
#include "verify.c"
#include "auth.c"

void * init_data() {
    return malloc(1);
}

obfs * new_obfs() {
    obfs * self = (obfs*)malloc(sizeof(obfs));
    self->l_data = NULL;
    return self;
}

void set_server_info(obfs *self, server_info *server) {
    memmove(&self->server, server, sizeof(server_info));
}

void get_server_info(obfs *self, server_info *server) {
    memmove(server, &self->server, sizeof(server_info));
}

void dispose_obfs(obfs *self) {
    free(self);
}

obfs_class * new_obfs_class(char *plugin_name)
{
    if (plugin_name == NULL)
        return NULL;
    if (strcmp(plugin_name, "origin") == 0)
        return NULL;
    if (strcmp(plugin_name, "plain") == 0)
        return NULL;
    init_crc32_table();
    init_shift128plus();
    if (strcmp(plugin_name, "http_simple") == 0) {
        obfs_class * plugin = (obfs_class*)malloc(sizeof(obfs));
        plugin->init_data = init_data;
        plugin->new_obfs = http_simple_new_obfs;
        plugin->get_server_info = get_server_info;
        plugin->set_server_info = set_server_info;
        plugin->dispose = http_simple_dispose;

        plugin->client_encode = http_simple_client_encode;
        plugin->client_decode = http_simple_client_decode;

        plugin->server_encode = http_simple_server_encode;
        plugin->server_decode = http_simple_server_decode;

        return plugin;
    } else if (strcmp(plugin_name, "http_post") == 0) {
        obfs_class * plugin = (obfs_class*)malloc(sizeof(obfs));
        plugin->init_data = init_data;
        plugin->new_obfs = http_simple_new_obfs;
        plugin->get_server_info = get_server_info;
        plugin->set_server_info = set_server_info;
        plugin->dispose = http_simple_dispose;

        plugin->client_encode = http_post_client_encode;
        plugin->client_decode = http_simple_client_decode;

        plugin->server_encode = http_simple_server_encode;
        plugin->server_decode = http_simple_server_decode;

        return plugin;
    } else if (strcmp(plugin_name, "tls1.2_ticket_auth") == 0) {
        obfs_class * plugin = (obfs_class*)malloc(sizeof(obfs));
        plugin->init_data = tls12_ticket_auth_init_data;
        plugin->new_obfs = tls12_ticket_auth_new_obfs;
        plugin->get_server_info = get_server_info;
        plugin->set_server_info = set_server_info;
        plugin->dispose = tls12_ticket_auth_dispose;

        plugin->client_encode = tls12_ticket_auth_client_encode;
        plugin->client_decode = tls12_ticket_auth_client_decode;

        plugin->server_encode = tls12_ticket_auth_server_encode;
        plugin->server_decode = tls12_ticket_auth_server_decode;

        return plugin;
    } else if (strcmp(plugin_name, "verify_simple") == 0) {
        obfs_class * plugin = (obfs_class*)malloc(sizeof(obfs));
        plugin->init_data = init_data;
        plugin->new_obfs = verify_simple_new_obfs;
        plugin->get_server_info = get_server_info;
        plugin->set_server_info = set_server_info;
        plugin->dispose = verify_simple_dispose;

        plugin->client_pre_encrypt = verify_simple_client_pre_encrypt;
        plugin->client_post_decrypt = verify_simple_client_post_decrypt;
        plugin->client_udp_pre_encrypt = NULL;
        plugin->client_udp_post_decrypt = NULL;

        plugin->server_pre_encrypt = verify_simple_server_pre_encrypt;
        plugin->server_post_decrypt = verify_simple_server_post_decrypt;
        plugin->server_udp_pre_encrypt = NULL;
        plugin->server_udp_post_decrypt = NULL;

        return plugin;
    } else if (strcmp(plugin_name, "auth_simple") == 0) {
        obfs_class * plugin = (obfs_class*)malloc(sizeof(obfs));
        plugin->init_data = auth_simple_init_data;
        plugin->new_obfs = auth_simple_new_obfs;
        plugin->get_server_info = get_server_info;
        plugin->set_server_info = set_server_info;
        plugin->dispose = auth_simple_dispose;

        plugin->client_pre_encrypt = auth_simple_client_pre_encrypt;
        plugin->client_post_decrypt = auth_simple_client_post_decrypt;
        plugin->client_udp_pre_encrypt = NULL;
        plugin->client_udp_post_decrypt = NULL;

        return plugin;
    } else if (strcmp(plugin_name, "auth_sha1") == 0) {
        obfs_class * plugin = (obfs_class*)malloc(sizeof(obfs));
        plugin->init_data = auth_simple_init_data;
        plugin->new_obfs = auth_simple_new_obfs;
        plugin->get_server_info = get_server_info;
        plugin->set_server_info = set_server_info;
        plugin->dispose = auth_simple_dispose;

        plugin->client_pre_encrypt = auth_sha1_client_pre_encrypt;
        plugin->client_post_decrypt = auth_sha1_client_post_decrypt;
        plugin->client_udp_pre_encrypt = NULL;
        plugin->client_udp_post_decrypt = NULL;

        return plugin;
    } else if (strcmp(plugin_name, "auth_sha1_v2") == 0) {
        obfs_class * plugin = (obfs_class*)malloc(sizeof(obfs));
        plugin->init_data = auth_simple_init_data;
        plugin->new_obfs = auth_simple_new_obfs;
        plugin->get_server_info = get_server_info;
        plugin->set_server_info = set_server_info;
        plugin->dispose = auth_simple_dispose;

        plugin->client_pre_encrypt = auth_sha1_v2_client_pre_encrypt;
        plugin->client_post_decrypt = auth_sha1_v2_client_post_decrypt;
        plugin->client_udp_pre_encrypt = NULL;
        plugin->client_udp_post_decrypt = NULL;

        return plugin;
    } else if (strcmp(plugin_name, "auth_sha1_v4") == 0) {
        obfs_class * plugin = (obfs_class*)malloc(sizeof(obfs));
        plugin->init_data = auth_simple_init_data;
        plugin->new_obfs = auth_simple_new_obfs;
        plugin->get_server_info = get_server_info;
        plugin->set_server_info = set_server_info;
        plugin->dispose = auth_simple_dispose;

        plugin->client_pre_encrypt = auth_sha1_v4_client_pre_encrypt;
        plugin->client_post_decrypt = auth_sha1_v4_client_post_decrypt;
        plugin->client_udp_pre_encrypt = NULL;
        plugin->client_udp_post_decrypt = NULL;

        return plugin;
    } else if (strcmp(plugin_name, "auth_aes128_md5") == 0) {
        obfs_class * plugin = (obfs_class*)malloc(sizeof(obfs));
        plugin->init_data = auth_simple_init_data;
        plugin->new_obfs = auth_aes128_md5_new_obfs;
        plugin->get_server_info = get_server_info;
        plugin->set_server_info = set_server_info;
        plugin->dispose = auth_simple_dispose;

        plugin->client_pre_encrypt = auth_aes128_sha1_client_pre_encrypt;
        plugin->client_post_decrypt = auth_aes128_sha1_client_post_decrypt;
        plugin->client_udp_pre_encrypt = auth_aes128_sha1_client_udp_pre_encrypt;
        plugin->client_udp_post_decrypt = auth_aes128_sha1_client_udp_post_decrypt;

        return plugin;
    } else if (strcmp(plugin_name, "auth_aes128_sha1") == 0) {
        obfs_class * plugin = (obfs_class*)malloc(sizeof(obfs));
        plugin->init_data = auth_simple_init_data;
        plugin->new_obfs = auth_aes128_sha1_new_obfs;
        plugin->get_server_info = get_server_info;
        plugin->set_server_info = set_server_info;
        plugin->dispose = auth_simple_dispose;

        plugin->client_pre_encrypt = auth_aes128_sha1_client_pre_encrypt;
        plugin->client_post_decrypt = auth_aes128_sha1_client_post_decrypt;
        plugin->client_udp_pre_encrypt = auth_aes128_sha1_client_udp_pre_encrypt;
        plugin->client_udp_post_decrypt = auth_aes128_sha1_client_udp_post_decrypt;

        return plugin;
    }
    LOGE("Load obfs '%s' failed", plugin_name);
    return NULL;
}

void free_obfs_class(obfs_class *plugin) {
    free(plugin);
}
