/* -*- Mode: C; tab-width: 4; -*- */
/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：mystate.c
* 摘	要：改变认证状态
* 作	者：HustMoon@BYHH
* 邮	箱：www.ehust@gmail.com
*/
#include "mystate.h"
#include "myfunc.h"
#include "dlfunc.h"
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>

#define MAX_SEND_COUNT		3	/* 最大超时次数 */

volatile int state = ID_DISCONNECT;	/* 认证状态 */
const u_char *capBuf = NULL;	/* 抓到的包 */
static u_char sendPacket[0x3E8];	/* 用来发送的包 */
static int sendCount = 0;	/* 同一阶段发包计数 */

extern const u_char STANDARD_ADDR[];
extern char userName[];
extern unsigned startMode;
extern unsigned dhcpMode;
extern u_char localMAC[], destMAC[];
extern unsigned timeout;
extern unsigned echoInterval;
extern unsigned restartWait;
extern char dhcpScript[];
extern pcap_t *hPcap;
extern u_char *fillBuf;
extern unsigned fillSize;
extern u_int32_t pingHost;
#ifndef NO_ARP
extern u_int32_t rip, gateway;
extern u_char gateMAC[];
static void sendArpPacket();	/* ARP监视 */
#endif

static void setTimer(unsigned interval);	/* 设置定时器 */
static int renewIP();	/* 更新IP */
static void fillEtherAddr(u_int32_t protocol);  /* 填充MAC地址和协议 */
static int sendStartPacket();	 /* 发送Start包 */
static int sendIdentityPacket();	/* 发送Identity包 */
static int sendChallengePacket();   /* 发送Md5 Challenge包 */
static int sendEchoPacket();	/* 发送心跳包 */
static int sendLogoffPacket();  /* 发送退出包 */
static int waitEchoPacket();	/* 等候响应包 */

static void setTimer(unsigned interval) /* 设置定时器 */
{
	struct itimerval timer;
	timer.it_value.tv_sec = interval;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = interval;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
}

int switchState(int type)
{
	if (state == type) /* 跟上次是同一状态？ */
		sendCount++;
	else
	{
		state = type;
		sendCount = 0;
	}
	if (sendCount>=MAX_SEND_COUNT && type!=ID_ECHO)  /* 超时太多次？ */
	{
		switch (type)
		{
		case ID_START:
			printf(">> 找不到服务器，重启认证!\n");
			break;
		case ID_IDENTITY:
			printf(">> 发送用户名超时，重启认证!\n");
			break;
		case ID_CHALLENGE:
			printf(">> 发送密码超时，重启认证!\n");
			break;
		case ID_WAITECHO:
			printf(">> 等候响应包超时，自行响应!\n");
			return switchState(ID_ECHO);
		}
		return restart();
	}
	switch (type)
	{
	case ID_DHCP:
		return renewIP();
	case ID_START:
		return sendStartPacket();
	case ID_IDENTITY:
		return sendIdentityPacket();
	case ID_CHALLENGE:
		return sendChallengePacket();
	case ID_WAITECHO:	/* 塞尔的就不ping了，不好计时 */
		return waitEchoPacket();
	case ID_ECHO:
		if (pingHost && sendCount*echoInterval > 60) {	/* 1分钟左右 */
			if (isOnline() == -1) {
				printf(">> 认证掉线，开始重连!\n");
				return switchState(ID_START);
			}
			sendCount = 1;
		}
#ifndef NO_ARP
		if (gateMAC[0] != 0xFE)
			sendArpPacket();
#endif
		return sendEchoPacket();
	case ID_DISCONNECT:
		return sendLogoffPacket();
	}
	return 0;
}

int restart()
{
	if (startMode >= 3)	/* 标记服务器地址为未获取 */
		startMode -= 3;
	state = ID_START;
	sendCount = -1;
	setTimer(restartWait);	/* restartWait秒后或者服务器请求后重启认证 */
	return 0;
}

static int renewIP()
{
	setTimer(0);	/* 取消定时器 */
	printf(">> 正在获取IP...\n");
	system(dhcpScript);
	printf(">> 操作结束。\n");
	dhcpMode += 3; /* 标记为已获取，123变为456，5不需再认证*/
	if (fillHeader() == -1)
		exit(EXIT_FAILURE);
	if (dhcpMode == 5)
		return switchState(ID_ECHO);
	return switchState(ID_START);
}

static void fillEtherAddr(u_int32_t protocol)
{
	memset(sendPacket, 0, 0x3E8);
	memcpy(sendPacket, destMAC, 6);
	memcpy(sendPacket+0x06, localMAC, 6);
	*(u_int32_t *)(sendPacket+0x0C) = htonl(protocol);
}

static int sendStartPacket()
{
	if (startMode%3 == 2)	/* 赛尔 */
	{
		if (sendCount == 0)
		{
			printf(">> 寻找服务器...\n");
			memcpy(sendPacket, STANDARD_ADDR, 6);
			memcpy(sendPacket+0x06, localMAC, 6);
			*(u_int32_t *)(sendPacket+0x0C) = htonl(0x888E0101);
			*(u_int16_t *)(sendPacket+0x10) = 0;
			memset(sendPacket+0x12, 0xa5, 42);
			setTimer(timeout);
		}
		return pcap_sendpacket(hPcap, sendPacket, 60);
	}
	if (sendCount == 0)
	{
		printf(">> 寻找服务器...\n");
		fillStartPacket();
		fillEtherAddr(0x888E0101);
		memcpy(sendPacket+0x12, fillBuf, fillSize);
		setTimer(timeout);
	}
	return pcap_sendpacket(hPcap, sendPacket, 0x3E8);
}

static int sendIdentityPacket()
{
	int nameLen = strlen(userName);
	if (startMode%3 == 2)	/* 赛尔 */
	{
		if (sendCount == 0)
		{
			printf(">> 发送用户名...\n");
			*(u_int16_t *)(sendPacket+0x0E) = htons(0x0100);
			*(u_int16_t *)(sendPacket+0x10) = *(u_int16_t *)(sendPacket+0x14) = htons(nameLen+30);
			sendPacket[0x12] = 0x02;
			sendPacket[0x16] = 0x01;
			sendPacket[0x17] = 0x01;
			fillCernetAddr(sendPacket);
			memcpy(sendPacket+0x28, "03.02.05", 8);
			memcpy(sendPacket+0x30, userName, nameLen);
			setTimer(timeout);
		}
		sendPacket[0x13] = capBuf[0x13];
		return pcap_sendpacket(hPcap, sendPacket, nameLen+48);
	}
	if (sendCount == 0)
	{
		printf(">> 发送用户名...\n");
		fillEtherAddr(0x888E0100);
		nameLen = strlen(userName);
		*(u_int16_t *)(sendPacket+0x14) = *(u_int16_t *)(sendPacket+0x10) = htons(nameLen+5);
		sendPacket[0x12] = 0x02;
		sendPacket[0x13] = capBuf[0x13];
		sendPacket[0x16] = 0x01;
		memcpy(sendPacket+0x17, userName, nameLen);
		memcpy(sendPacket+0x17+nameLen, fillBuf, fillSize);
		setTimer(timeout);
	}
	return pcap_sendpacket(hPcap, sendPacket, 0x3E8);
}

static int sendChallengePacket()
{
	int nameLen = strlen(userName);
	if (startMode%3 == 2)	/* 赛尔 */
	{
		if (sendCount == 0)
		{
			printf(">> 发送密码...\n");
			*(u_int16_t *)(sendPacket+0x0E) = htons(0x0100);
			*(u_int16_t *)(sendPacket+0x10) = *(u_int16_t *)(sendPacket+0x14) = htons(nameLen+22);
			sendPacket[0x12] = 0x02;
			sendPacket[0x13] = capBuf[0x13];
			sendPacket[0x16] = 0x04;
			sendPacket[0x17] = 16;
			memcpy(sendPacket+0x18, checkPass(capBuf[0x13], capBuf+0x18, capBuf[0x17]), 16);
			memcpy(sendPacket+0x28, userName, nameLen);
			setTimer(timeout);
		}
		return pcap_sendpacket(hPcap, sendPacket, nameLen+40);
	}
	if (sendCount == 0)
	{
		printf(">> 发送密码...\n");
		fillMd5Packet(capBuf+0x18);
		fillEtherAddr(0x888E0100);
		*(u_int16_t *)(sendPacket+0x14) = *(u_int16_t *)(sendPacket+0x10) = htons(nameLen+22);
		sendPacket[0x12] = 0x02;
		sendPacket[0x13] = capBuf[0x13];
		sendPacket[0x16] = 0x04;
		sendPacket[0x17] = 16;
		memcpy(sendPacket+0x18, checkPass(capBuf[0x13], capBuf+0x18, capBuf[0x17]), 16);
		memcpy(sendPacket+0x28, userName, nameLen);
		memcpy(sendPacket+0x28+nameLen, fillBuf, fillSize);
		setTimer(timeout);
	}
	return pcap_sendpacket(hPcap, sendPacket, 0x3E8);
}

static int sendEchoPacket()
{
	if (startMode%3 == 2)	/* 赛尔 */
	{
		*(u_int16_t *)(sendPacket+0x0E) = htons(0x0106);
		*(u_int16_t *)(sendPacket+0x10) = 0;
		memset(sendPacket+0x12, 0xa5, 42);
		switchState(ID_WAITECHO);	/* 继续等待 */
		return pcap_sendpacket(hPcap, sendPacket, 60);
	}
	if (sendCount == 0)
	{
		u_char echo[] =
		{
			0x00,0x1E,0xFF,0xFF,0x37,0x77,0x7F,0x9F,0xFF,0xFF,0xD9,0x13,0xFF,0xFF,0x37,0x77,
			0x7F,0x9F,0xFF,0xFF,0xF7,0x2B,0xFF,0xFF,0x37,0x77,0x7F,0x3F,0xFF
		};
		printf(">> 发送心跳包以保持在线...\n");
		fillEtherAddr(0x888E01BF);
		memcpy(sendPacket+0x10, echo, sizeof(echo));
		setTimer(echoInterval);
	}
	fillEchoPacket(sendPacket);
	return pcap_sendpacket(hPcap, sendPacket, 0x2D);
}

static int sendLogoffPacket()
{
	setTimer(0);	/* 取消定时器 */
	if (startMode%3 == 2)	/* 赛尔 */
	{
		*(u_int16_t *)(sendPacket+0x0E) = htons(0x0102);
		*(u_int16_t *)(sendPacket+0x10) = 0;
		memset(sendPacket+0x12, 0xa5, 42);
		return pcap_sendpacket(hPcap, sendPacket, 60);
	}
	fillStartPacket();	/* 锐捷的退出包与Start包类似，不过其实不这样也是没问题的 */
	fillEtherAddr(0x888E0102);
	memcpy(sendPacket+0x12, fillBuf, fillSize);
	return pcap_sendpacket(hPcap, sendPacket, 0x3E8);
}

static int waitEchoPacket()
{
	if (sendCount == 0)
		setTimer(echoInterval);
	return 0;
}

#ifndef NO_ARP
static void sendArpPacket()
{
	u_char arpPacket[0x3C] = {
		0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x06,0x00,0x01,
		0x08,0x00,0x06,0x04,0x00};

	if (gateMAC[0] != 0xFF) {
		memcpy(arpPacket, gateMAC, 6);
		memcpy(arpPacket+0x06, localMAC, 6);
		arpPacket[0x15]=0x02;
		memcpy(arpPacket+0x16, localMAC, 6);
		memcpy(arpPacket+0x1c, &rip, 4);
		memcpy(arpPacket+0x20, gateMAC, 6);
		memcpy(arpPacket+0x26, &gateway, 4);
		pcap_sendpacket(hPcap, arpPacket, 0x3C);
	}
	memset(arpPacket, 0xFF, 6);
	memcpy(arpPacket+0x06, localMAC, 6);
	arpPacket[0x15]=0x01;
	memcpy(arpPacket+0x16, localMAC, 6);
	memcpy(arpPacket+0x1c, &rip, 4);
	memset(arpPacket+0x20, 0, 6);
	memcpy(arpPacket+0x26, &gateway, 4);
	pcap_sendpacket(hPcap, arpPacket, 0x2A);
}
#endif
