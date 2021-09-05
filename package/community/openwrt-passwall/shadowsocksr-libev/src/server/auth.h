/*
 * auth.h - Define shadowsocksR server's buffers and callbacks
 *
 * Copyright (C) 2015 - 2016, Break Wa11 <mmgac001@gmail.com>
 */

#ifndef _AUTH_H
#define _AUTH_H

void * auth_simple_init_data();
obfs * auth_simple_new_obfs();
void auth_simple_dispose(obfs *self);

int auth_simple_client_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);
int auth_simple_client_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);


int auth_sha1_client_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);
int auth_sha1_client_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);

int auth_sha1_v2_client_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);
int auth_sha1_v2_client_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);

int auth_sha1_v4_client_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);
int auth_sha1_v4_client_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);

int auth_aes128_sha1_client_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);
int auth_aes128_sha1_client_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);

#endif // _AUTH_H
