/* -*- Mode: C; tab-width: 4; -*- */
/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：mentohust.c
* 摘	要：MentoHUST主函数
* 作	者：HustMoon@BYHH
* 邮	箱：www.ehust@gmail.com
*/

#include "strnormalize.h"

#include "myconfig.h"
#include "mystate.h"
#include "myfunc.h"
#include "dlfunc.h"

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>

extern pcap_t *hPcap;
extern volatile int state;
extern u_char *fillBuf;
extern const u_char *capBuf;
extern unsigned startMode, dhcpMode, maxFail;
extern u_char destMAC[];
extern int lockfd;
#ifndef NO_NOTIFY
extern int showNotify;
#endif
#ifndef NO_ARP
extern u_int32_t rip, gateway;
extern u_char gateMAC[];
#endif

static void exit_handle(void);	/* 退出回调 */
static void sig_handle(int sig);	/* 信号回调 */
static void pcap_handle(u_char *user, const struct pcap_pkthdr *h, const u_char *buf);	/* pcap_loop回调 */
static void showRuijieMsg(const u_char *buf, unsigned bufLen);	/* 显示锐捷服务器提示信息 */
static void showCernetMsg(const u_char *buf);	/* 显示赛尔服务器提示信息 */

int main(int argc, char **argv)
{
	atexit(exit_handle);
	initConfig(argc, argv);
	signal(SIGALRM, sig_handle);	/* 定时器 */
	signal(SIGHUP, sig_handle);	 /* 注销时 */
	signal(SIGINT, sig_handle);	 /* Ctrl+C */
	signal(SIGQUIT, sig_handle);	/* Ctrl+\ */
	signal(SIGTSTP, sig_handle);	/* Ctrl+Z */
	signal(SIGTERM, sig_handle);	/* 被结束时 */
	if (dhcpMode == 3)	  /* 认证前DHCP */
		switchState(ID_DHCP);
	else
		switchState(ID_START);	/* 开始认证 */
	if (-1 == pcap_loop(hPcap, -1, pcap_handle, NULL)) { /* 开始捕获数据包 */
		printf("!! 捕获数据包失败，请检查网络连接！\n");
#ifndef NO_NOTIFY
		if (showNotify)
			show_notify("MentoHUST - 错误提示", "捕获数据包失败，请检查网络连接！");
#endif
	}
	exit(EXIT_FAILURE);
}

static void exit_handle(void)
{
	if (state != ID_DISCONNECT)
		switchState(ID_DISCONNECT);
	if (hPcap != NULL)
		pcap_close(hPcap);
	if (fillBuf != NULL)
		free(fillBuf);
	if (lockfd > -1)
		close(lockfd);
#ifndef NO_NOTIFY
	free_libnotify();
#endif
#ifndef NO_DYLOAD
	free_libpcap();
#endif
	printf(">> 认证已退出。\n");
}

static void sig_handle(int sig)
{
	if (sig == SIGALRM)	 /* 定时器 */
	{
		if (-1 == switchState(state))
		{
			pcap_breakloop(hPcap);
			printf("!! 发送数据包失败, 请检查网络连接！\n");
#ifndef NO_NOTIFY
			if (showNotify)
				show_notify("MentoHUST - 错误提示", "发送数据包失败, 请检查网络连接！");
#endif
			exit(EXIT_FAILURE);
		}
	}
	else	/* 退出 */
	{
		pcap_breakloop(hPcap);
		exit(EXIT_SUCCESS);
	}
}

static void pcap_handle(u_char *user, const struct pcap_pkthdr *h, const u_char *buf)
{
	static unsigned failCount = 0;
#ifndef NO_ARP
	if (buf[0x0c]==0x88 && buf[0x0d]==0x8e) {
#endif
		if (memcmp(destMAC, buf+6, 6)!=0 && startMode>2)	/* 服务器MAC地址不符 */
			return;
		capBuf = buf;
		if (buf[0x0F]==0x00 && buf[0x12]==0x01 && buf[0x16]==0x01) {	/* 验证用户名 */
			if (startMode < 3) {
				memcpy(destMAC, buf+6, 6);
				printf("** 认证MAC:\t%s\n", formatHex(destMAC, 6));
				startMode += 3;	/* 标记为已获取 */
			}
			if (startMode==3 && memcmp(buf+0x17, "User name", 9)==0)	/* 塞尔 */
				startMode = 5;
			switchState(ID_IDENTITY);
		}
		else if (buf[0x0F]==0x00 && buf[0x12]==0x01 && buf[0x16]==0x04)	/* 验证密码 */
			switchState(ID_CHALLENGE);
		else if (buf[0x0F]==0x00 && buf[0x12]==0x03) {	/* 认证成功 */
			printf(">> 认证成功!\n");
			failCount = 0;
			if (!(startMode%3 == 2)) {
				getEchoKey(buf);
				showRuijieMsg(buf, h->caplen);
			}
			if (dhcpMode==1 || dhcpMode==2)	/* 二次认证第一次或者认证后 */
				switchState(ID_DHCP);
			else if (startMode%3 == 2)
				switchState(ID_WAITECHO);
			else
				switchState(ID_ECHO);
		}
		else if (buf[0x0F]==0x00 && buf[0x12]==0x01 && buf[0x16]==0x02)	/* 显示赛尔提示信息 */
			showCernetMsg(buf);
		else if (buf[0x0F] == 0x05)	/* (赛尔)响应在线 */
			switchState(ID_ECHO);
		else if (buf[0x0F]==0x00 && buf[0x12]==0x04) {  /* 认证失败或被踢下线 */
			if (state==ID_WAITECHO || state==ID_ECHO) {
				printf(">> 认证掉线，开始重连!\n");
				switchState(ID_START);
			}
			else if (buf[0x1b]!=0 || startMode%3==2) {
				printf(">> 认证失败!\n");
				if (startMode%3 != 2)
					showRuijieMsg(buf, h->caplen);
				if (maxFail && ++failCount>=maxFail) {
					printf(">> 连续认证失败%u次，退出认证。\n", maxFail);
					exit(EXIT_SUCCESS);
				}
				restart();
			}
			else
				switchState(ID_START);
		}
#ifndef NO_ARP
	} else if (gateMAC[0]!=0xFE && buf[0x0c]==0x08 && buf[0x0d]==0x06) {
		if (*(u_int32_t *)(buf+0x1c) == gateway) {
			char str[50];
			if (gateMAC[0] == 0xFF) {
				memcpy(gateMAC, buf+0x16, 6);
				printf("** 网关MAC:\t%s\n", formatHex(gateMAC, 6));
				fflush(stdout);
				sprintf(str, "arp -s %s %s", formatIP(gateway), formatHex(gateMAC, 6));
				system(str);
			} else if (buf[0x15]==0x02 && *(u_int32_t *)(buf+0x26)==rip
				&& memcmp(gateMAC, buf+0x16, 6)!=0) {
				printf("** ARP欺骗:\t%s\n", formatHex(buf+0x16, 6));
				fflush(stdout);
#ifndef NO_NOTIFY
				if (showNotify) {
					sprintf(str, "欺骗源: %s", formatHex(buf+0x16, 6));
					show_notify("MentoHUST - ARP提示", str);
				}
#endif
			}
		}
	}
#endif
}

#ifndef MAC_OS
static char *gbk2utf(char *gbksrc, size_t gbklen)	/* GBK转UTF－8 */
#else
static char *gbk2utf(const char *gbksrc, size_t gbklen)	/* GBK转UTF－8 */
#endif
{

	/* GBK一汉字俩字节，UTF-8一汉字3字节，二者ASCII字符均一字节
		 所以这样申请是足够的了，要记得释放 */
	str_normalize_init();

	size_t utf8len = gbklen * 3 + 1;
	char *utf8dst = (char *)malloc(utf8len);

	memset(utf8dst,0,utf8len);
	
	char *temp=(char *)malloc(gbklen+5);
	memset(temp, 0, gbklen+5);
	memcpy(temp,gbksrc,gbklen);
	gbksrc = temp;
	gbklen = strlen(gbksrc);

	gbk_to_utf8(gbksrc, gbklen, &utf8dst, &utf8len);

	free(temp);

	return utf8dst;
}

static void showRuijieMsg(const u_char *buf, unsigned bufLen)
{
	char *serverMsg;
	int length = buf[0x1b];
	if (length > 0)
	{
		for (serverMsg=(char *)(buf+0x1c); *serverMsg=='\r'||*serverMsg=='\n'; serverMsg++,length--);	/* 跳过开头的换行符 */
		if (strlen(serverMsg) < length)
			length = strlen(serverMsg);
		if (length>0 && (serverMsg=gbk2utf(serverMsg, length))!=NULL)
		{
			printf("$$ 系统提示:\t%s\n", serverMsg);
#ifndef NO_NOTIFY
			if (showNotify)
				show_notify("MentoHUST - 系统提示", serverMsg);
#endif
			free(serverMsg);
		}
	}
	if ((length=0x1c+buf[0x1b]+0x69+39) < bufLen)
	{
		serverMsg=(char *)(buf+length);
		if (buf[length-1]-2 > bufLen-length)
			length = bufLen - length;
		else
			length = buf[length-1]-2;
		for (; *serverMsg=='\r'||*serverMsg=='\n'; serverMsg++,length--);
		if (length>0 && (serverMsg=gbk2utf(serverMsg, length))!=NULL)
		{
			printf("$$ 计费提示:\t%s\n", serverMsg);
#ifndef NO_NOTIFY
			if (showNotify)
				show_notify("MentoHUST - 计费提示", serverMsg);
#endif
			free(serverMsg);
		}
	}
	fflush(stdout);
}

static void showCernetMsg(const u_char *buf)
{
	char *serverMsg = (char *)(buf+0x17);
	int length = ntohs(*(u_int16_t *)(buf+0x14)) - 5;
	if (strlen(serverMsg) < length)
		length = strlen(serverMsg);
	if (length>0 && (serverMsg=gbk2utf(serverMsg, length))!=NULL)
	{
		printf("$$ 系统提示:\t%s\n", serverMsg);
#ifndef NO_NOTIFY
			if (showNotify)
				show_notify("MentoHUST - 系统提示", serverMsg);
#endif
		free(serverMsg);
	}
	fflush(stdout);
}
