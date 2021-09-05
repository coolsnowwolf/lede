/*
 * owipcalc - OpenWrt IP Calculator
 *
 *   Copyright (C) 2012 Jo-Philipp Wich <jo@mein.io>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#ifndef __OWIPCALC_H__
#define __OWIPCALC_H__

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <arpa/inet.h>

#define qprintf(...) \
	do { \
		if (!quiet) printf(__VA_ARGS__); \
		printed = true; \
	} while(0)

struct cidr {
	uint8_t family;
	uint32_t prefix;
	union {
		struct in_addr v4;
		struct in6_addr v6;
	} addr;
	union {
		char v4[sizeof("255.255.255.255/255.255.255.255 ")];
		char v6[sizeof("FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:255.255.255.255/128 ")];
	} buf;
	struct cidr *next;
};

void cidr_push(struct cidr *a);
bool cidr_pop(struct cidr *a);

struct cidr * cidr_parse4(const char *s);
bool cidr_add4(struct cidr *a, struct cidr *b);
bool cidr_sub4(struct cidr *a, struct cidr *b);
bool cidr_network4(struct cidr *a);
bool cidr_broadcast4(struct cidr *a);
bool cidr_contains4(struct cidr *a, struct cidr *b);
bool cidr_netmask4(struct cidr *a);
bool cidr_private4(struct cidr *a);
bool cidr_linklocal4(struct cidr *a);
bool cidr_prev4(struct cidr *a, struct cidr *b);
bool cidr_next4(struct cidr *a, struct cidr *b);
bool cidr_6to4(struct cidr *a);
bool cidr_print4(struct cidr *a);
struct cidr * cidr_parse6(const char *s);
bool cidr_add6(struct cidr *a, struct cidr *b);
bool cidr_sub6(struct cidr *a, struct cidr *b);
bool cidr_prev6(struct cidr *a, struct cidr *b);
bool cidr_next6(struct cidr *a, struct cidr *b);
bool cidr_network6(struct cidr *a);
bool cidr_contains6(struct cidr *a, struct cidr *b);
bool cidr_linklocal6(struct cidr *a);
bool cidr_ula6(struct cidr *a);
bool cidr_print6(struct cidr *a);
struct cidr * cidr_parse(const char *op, const char *s, int af_hint);
bool cidr_howmany(struct cidr *a, struct cidr *b);
bool cidr_prefix(struct cidr *a, struct cidr *b);
bool cidr_quiet(struct cidr *a);

extern struct cidr *stack;
extern bool quiet;
extern bool printed;

#endif
