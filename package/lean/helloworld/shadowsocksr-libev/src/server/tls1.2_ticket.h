/*
 * http_simple.h - Define shadowsocksR server's buffers and callbacks
 *
 * Copyright (C) 2015 - 2016, Break Wa11 <mmgac001@gmail.com>
 */

#ifndef _TLS1_2_TICKET_H
#define _TLS1_2_TICKET_H

void * tls12_ticket_auth_init_data();
obfs * tls12_ticket_auth_new_obfs();
void tls12_ticket_auth_dispose(obfs *self);

int tls12_ticket_auth_client_encode(obfs *self, char **pencryptdata, int datalength, size_t* capacity);
int tls12_ticket_auth_client_decode(obfs *self, char **pencryptdata, int datalength, size_t* capacity, int *needsendback);

int tls12_ticket_auth_server_encode(obfs *self, char **pencryptdata, int datalength, size_t* capacity);
int tls12_ticket_auth_server_decode(obfs *self, char **pencryptdata, int datalength, size_t* capacity, int *needsendback);

#endif // _TLS1_2_TICKET_H
