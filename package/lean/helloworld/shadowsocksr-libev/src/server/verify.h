/*
 * verify.h - Define shadowsocksR server's buffers and callbacks
 *
 * Copyright (C) 2015 - 2016, Break Wa11 <mmgac001@gmail.com>
 */

#ifndef _VERIFY_H
#define _VERIFY_H

obfs * verify_simple_new_obfs();
void verify_simple_dispose(obfs *self);

int verify_simple_client_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);
int verify_simple_client_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);

int verify_simple_server_pre_encrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);
int verify_simple_server_post_decrypt(obfs *self, char **pplaindata, int datalength, size_t* capacity);

#endif // _VERIFY_H
