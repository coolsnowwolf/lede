/* -*- Mode: C; tab-width: 4; -*- */
/*
* 文件名称：dlfunc.h
* 摘	要：动态载入库函数
*/
#ifndef HUSTMOON_DLFUNC_H
#define HUSTMOON_DLFUNC_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <sys/time.h>

#define PCAP_ERRBUF_SIZE	256
#define PCAP_IF_LOOPBACK	0x00000001

typedef unsigned int bpf_u_int32;
typedef void pcap_t;
typedef struct pcap_if {
	struct pcap_if *next;
	char *name;
	char *description;
	void *addresses;
	bpf_u_int32 flags;
}pcap_if_t;
struct bpf_program {
	unsigned int bf_len;
	void *bf_insns;
};
struct pcap_pkthdr {
	struct timeval ts;
	bpf_u_int32 caplen;
	bpf_u_int32 len;
};
typedef void (*pcap_handler)(unsigned char *, const struct pcap_pkthdr *, const unsigned char *);

#ifdef NO_DYLOAD
int pcap_findalldevs(pcap_if_t **, char *);
void pcap_freealldevs(pcap_if_t *);
pcap_t *pcap_open_live(const char *, int, int, int, char *);
int pcap_compile(pcap_t *, struct bpf_program *, const char *, int, bpf_u_int32);
int pcap_setfilter(pcap_t *, struct bpf_program *);
char *pcap_geterr(pcap_t *);
void pcap_freecode(struct bpf_program *);
int pcap_loop(pcap_t *, int, pcap_handler, unsigned char *);
void pcap_close(pcap_t *);
void pcap_breakloop(pcap_t *);
int pcap_sendpacket(pcap_t *, const unsigned char *, int);

#else
extern int (*pcap_findalldevs)(pcap_if_t **, char *);
extern void (*pcap_freealldevs)(pcap_if_t *);
extern pcap_t *(*pcap_open_live)(const char *, int, int, int, char *);
extern int (*pcap_compile)(pcap_t *, struct bpf_program *, const char *, int, bpf_u_int32);
extern int (*pcap_setfilter)(pcap_t *, struct bpf_program *);
extern char *(*pcap_geterr)(pcap_t *);
extern void (*pcap_freecode)(struct bpf_program *);
extern int (*pcap_loop)(pcap_t *, int, pcap_handler, unsigned char *);
extern void (*pcap_close)(pcap_t *);
extern void (*pcap_breakloop)(pcap_t *);
extern int (*pcap_sendpacket)(pcap_t *, const unsigned char *, int);

int load_libpcap(void);	/* 载入libpcap.so */
void free_libpcap(void);	/* 释放libpcap.so */

#endif	/* NO_DYLOAD */

#ifndef NO_NOTIFY
int load_libnotify(void);	/* 载入libnotify.so */
void free_libnotify(void);	/* 释放libnotify.so */
void set_timeout(int timeout);	/* 设置超时间隔 */
void show_notify(const char *summary, char *body);	/* 显示通知:概要、正文 */
#endif	/* NO_NOTIFY */

#endif	/* HUSTMOON_DLFUNC_H */

