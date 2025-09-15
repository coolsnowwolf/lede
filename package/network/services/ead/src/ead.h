/*
 * Copyright (C) 2008 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __EAD_H
#define __EAD_H

#define EAD_DEBUGLEVEL	1

#include <stdint.h>
#include <stddef.h>

#ifndef MAXSALTLEN
#define MAXSALTLEN 32
#endif

#define EAD_PORT	56026UL
#define EAD_MAGIC	3671771902UL
#define EAD_CMD_TIMEOUT	10

#define EAD_MAX_IV_INCR	128

/* request/response types */
/* response id == request id + 1 */
enum ead_type {
	EAD_TYPE_PING,
	EAD_TYPE_PONG,

	EAD_TYPE_SET_USERNAME,
	EAD_TYPE_ACK_USERNAME,

	EAD_TYPE_GET_PRIME,
	EAD_TYPE_PRIME,

	EAD_TYPE_SEND_A,
	EAD_TYPE_SEND_B,

	EAD_TYPE_SEND_AUTH,
	EAD_TYPE_DONE_AUTH,

	EAD_TYPE_SEND_CMD,
	EAD_TYPE_RESULT_CMD,

	EAD_TYPE_LAST
};

enum ead_auth_type {
	EAD_AUTH_DEFAULT,
	EAD_AUTH_MD5
};

enum ead_cmd_type {
	EAD_CMD_NORMAL,
	EAD_CMD_BACKGROUND,
	EAD_CMD_LAST
};

struct ead_msg_pong {
	uint16_t auth_type;
	char name[];
} __attribute__((packed));

struct ead_msg_number {
	uint8_t id;
	unsigned char data[];
} __attribute__((packed));

struct ead_msg_salt {
	uint8_t prime;
	uint8_t len;
	unsigned char salt[MAXSALTLEN];
	unsigned char ext_salt[MAXSALTLEN];
} __attribute__((packed));

struct ead_msg_user {
	char username[32];
} __attribute__((packed));

struct ead_msg_auth {
	unsigned char data[20];
} __attribute__((packed));

struct ead_msg_cmd {
	uint8_t type;
	uint16_t timeout;
	unsigned char data[];
} __attribute__((packed));

struct ead_msg_cmd_data {
	uint8_t done;
	unsigned char data[];
} __attribute__((packed));

struct ead_msg_encrypted {
	uint32_t hash[5];
	uint32_t iv;
	uint8_t pad;
	union {
		struct ead_msg_cmd cmd;
		struct ead_msg_cmd_data cmd_data;
	} data[];
} __attribute__((packed));


#define EAD_DATA(_msg, _type) (&((_msg)->data[0]._type))
#define EAD_ENC_DATA(_msg, _type) (&((_msg)->data[0].enc.data[0]._type))

/* for ead_msg::sid */
#define EAD_INSTANCE_MASK	0xf000
#define EAD_INSTANCE_SHIFT	12

struct ead_msg {
	uint32_t magic;
	uint32_t len;
	uint32_t type;
	uint16_t nid; /* node id */
	uint16_t sid; /* session id */
	uint32_t ip; /* source ip for responses from the server */
	union {
		struct ead_msg_pong pong;
		struct ead_msg_user user;
		struct ead_msg_number number;
		struct ead_msg_auth auth;
		struct ead_msg_salt salt;
		struct ead_msg_encrypted enc;
	} data[];
} __attribute__((packed));


#endif
