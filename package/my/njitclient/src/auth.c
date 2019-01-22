/* File: auth.c
 * ------------
 * 注：核心函数为Authentication()，由该函数执行801.1X认证
 */

int Authentication(const char *UserName, const char *Password, const char *DeviceName, const char *Version, const char *Key);

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "h3c_AES_MD5.h"
#include <stdbool.h>

#include <pcap.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "debug.h"

// 自定义常量
typedef enum { REQUEST = 1,
			   RESPONSE = 2,
			   SUCCESS = 3,
			   FAILURE = 4,
			   H3CDATA = 10 } EAP_Code;
typedef enum { IDENTITY = 1,
			   NOTIFICATION = 2,
			   MD5 = 4,
			   ALLOCATED = 7,
			   AVAILABLE = 20 } EAP_Type;
typedef uint8_t EAP_ID;
const uint8_t BroadcastAddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // 广播MAC地址
const uint8_t MultcastAddr[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};  // 多播MAC地址
char H3C_VERSION[16];
char H3C_KEY[64];
//const char H3C_VERSION[16]="CH\x11V7.10-0313"; // 华为客户端版本号
//const char H3C_KEY[64]    ="HuaWei3COM1X";  // H3C的固定密钥
//const char H3C_KEY[64]  ="Oly5D62FaE94W7";  // H3C的另一个固定密钥，网友取自MacOSX版本的iNode官方客户端

uint8_t AES_MD5req[32];
uint8_t AES_MD5data[32];
// 子函数声明
static void SendStartPkt(pcap_t *adhandle, const uint8_t mac[]);
static void SendLogoffPkt(pcap_t *adhandle, const uint8_t mac[]);
static void SendResponseIdentity(pcap_t *adhandle,
								 const uint8_t request[],
								 const uint8_t ethhdr[],
								 const uint8_t ip[4],
								 const char username[]);

static void SendFirstResponseIdentity(pcap_t *adhandle,
									  const uint8_t request[],
									  const uint8_t ethhdr[],
									  const uint8_t ip[4],
									  const char username[]);
static void SendResponseMD5(pcap_t *adhandle,
							const uint8_t request[],
							const uint8_t ethhdr[],
							const char username[],
							const char passwd[]);
static void SendResponsePassword(pcap_t *handle,
								 const uint8_t request[],
								 const uint8_t ethhdr[],
								 const char username[],
								 const char passwd[]);
static void SendResponseAvailable(pcap_t *adhandle,
								  const uint8_t request[],
								  const uint8_t ethhdr[],
								  const uint8_t ip[4],
								  const char username[]);
static void SendResponseNotification(pcap_t *handle,
									 const uint8_t request[],
									 const uint8_t ethhdr[]);

static void GetMacFromDevice(uint8_t mac[6], const char *devicename);

static void FillClientVersionArea(uint8_t area[]);
static void FillWindowsVersionArea(uint8_t area[]);
static void FillBase64Area(char area[]);
// From fillmd5.c
extern void FillMD5Area(uint8_t digest[],
						uint8_t id, const char passwd[], const uint8_t srcMD5[]);

// From ip.c
extern void GetIpFromDevice(uint8_t ip[4], const char DeviceName[]);

int mode = 1; //auto reconnect

/**
 * 函数：Authentication()
 *
 * 使用以太网进行802.1X认证(802.1X Authentication)
 * 该函数将不断循环，应答802.1X认证会话，直到遇到错误后才退出
 */

int Authentication(const char *UserName, const char *Password, const char *DeviceName, const char *Version, const char *Key)
{
	strcpy(H3C_VERSION, Version);
	strcpy(H3C_KEY, Key);

	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *adhandle; // adapter handle
	uint8_t MAC[6];
	char FilterStr[100];
	struct bpf_program fcode;
	const int DefaultTimeout = 2000; //设置接收超时参数，单位ms

	// NOTE: 这里没有检查网线是否已插好,网线插口可能接触不良

	/* 打开适配器(网卡) */
	adhandle = pcap_open_live(DeviceName, 65536, 1, DefaultTimeout, errbuf);
	if (adhandle == NULL)
	{
		fprintf(stderr, "%s\n", errbuf);
		exit(-1);
	}

	/* 查询本机MAC地址 */
	GetMacFromDevice(MAC, DeviceName);

	/* 生成随机数 */
	srand((unsigned)time(NULL));

START_AUTHENTICATION:
{
	/*
		 * 设置过滤器：
		 * 初始情况下只捕获发往本机的802.1X认证会话，不接收多播信息（避免误捕获其他客户端发出的多播信息）
		 * 进入循环体前可以重设过滤器，那时再开始接收多播信息
		 */
	sprintf(FilterStr, "(ether proto 0x888e) and (ether dst host %02x:%02x:%02x:%02x:%02x:%02x)",
			MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
	pcap_compile(adhandle, &fcode, FilterStr, 1, 0xff);
	pcap_setfilter(adhandle, &fcode);
	int i;
	int retcode;
	struct pcap_pkthdr *header = NULL;
	const uint8_t *captured = NULL;
	uint8_t ethhdr[14] = {0};	  // ethernet header
	uint8_t ip[4] = {10, 0, 0, 0}; // ip address

	// 使用本机MAC地址生成伪装IP地址。
	ip[1] = MAC[2] ^ (uint8_t)rand();
	ip[2] = MAC[4] ^ (uint8_t)rand();
	ip[3] = MAC[5] ^ (uint8_t)rand();

	/* 主动发起认证会话 */
	SendStartPkt(adhandle, MAC);
	DPRINTF("[ ] Client: Start.\n");

	/* 等待认证服务器的回应 */
	bool serverIsFound = false;
	while (!serverIsFound)
	{
		retcode = pcap_next_ex(adhandle, &header, &captured);
		if (retcode == 1 && (EAP_Code)captured[18] == REQUEST)
			serverIsFound = true;
		else
		{ // 延时后重试
			sleep(1);
			//DPRINTF(".");
			SendStartPkt(adhandle, MAC);
			// NOTE: 这里没有检查网线是否接触不良或已被拔下
		}
	}

	// 填写应答包的报头(以后无须再修改)
	// 默认以单播方式应答802.1X认证设备发来的Request
	memcpy(ethhdr + 0, captured + 6, 6);
	memcpy(ethhdr + 6, MAC, 6);
	ethhdr[12] = 0x88;
	ethhdr[13] = 0x8e;

	// 收到的第一个包可能是Request Notification。取决于校方网络配置
	if ((EAP_Type)captured[22] == NOTIFICATION)
	{
		DPRINTF("[%d] Server: Request Notification!\n", captured[19]);
		// 发送Response Notification
		SendResponseNotification(adhandle, captured, ethhdr);
		DPRINTF("    Client: Response Notification.\n");

		// 继续接收下一个Request包
		retcode = pcap_next_ex(adhandle, &header, &captured);
		assert(retcode == 1);
		assert((EAP_Code)captured[18] == REQUEST);
	}

	// 分情况应答下一个包
	if ((EAP_Type)captured[22] == IDENTITY)
	{ // 通常情况会收到包Request Identity，应回答Response Identity
		DPRINTF("[%d] Server: Request Identity!\n", captured[19]);
		GetIpFromDevice(ip, DeviceName);
		SendFirstResponseIdentity(adhandle, captured, ethhdr, ip, UserName);
		DPRINTF("[%d] Client: Response First Identity.\n", (EAP_ID)captured[19]);
	}
	else if ((EAP_Type)captured[22] == AVAILABLE)
	{ // 遇到AVAILABLE包时需要特殊处理
		// 中南财经政法大学目前使用的格式：
		// 收到第一个Request AVAILABLE时要回答Response Identity
		DPRINTF("[%d] Server: Request AVAILABLE!\n", captured[19]);
		GetIpFromDevice(ip, DeviceName);
		SendResponseIdentity(adhandle, captured, ethhdr, ip, UserName);
		DPRINTF("[%d] Client: Response Identity.\n", (EAP_ID)captured[19]);
	}

	// 重设过滤器，只捕获华为802.1X认证设备发来的包（包括多播Request Identity / Request AVAILABLE）
	sprintf(FilterStr, "(ether proto 0x888e) and (ether src host %02x:%02x:%02x:%02x:%02x:%02x)",
			captured[6], captured[7], captured[8], captured[9], captured[10], captured[11]);
	pcap_compile(adhandle, &fcode, FilterStr, 1, 0xff);
	pcap_setfilter(adhandle, &fcode);

	// 进入循环体
	for (;;)
	{
		// 调用pcap_next_ex()函数捕获数据包
		while (pcap_next_ex(adhandle, &header, &captured) != 1)
		{
			//DPRINTF("."); // 若捕获失败，则等1秒后重试
			sleep(1); // 直到成功捕获到一个数据包后再跳出
					  // NOTE: 这里没有检查网线是否已被拔下或插口接触不良
		}

		// 根据收到的Request，回复相应的Response包
		if ((EAP_Code)captured[18] == REQUEST)
		{
			switch ((EAP_Type)captured[22])
			{
			case IDENTITY:
				DPRINTF("[%d] Server: Request Identity!\n", (EAP_ID)captured[19]);
				GetIpFromDevice(ip, DeviceName);
				SendResponseIdentity(adhandle, captured, ethhdr, ip, UserName);
				DPRINTF("[%d] Client: Response Identity.\n", (EAP_ID)captured[19]);
				break;
			case AVAILABLE:
				DPRINTF("[%d] Server: Request AVAILABLE!\n", (EAP_ID)captured[19]);
				GetIpFromDevice(ip, DeviceName);
				SendResponseAvailable(adhandle, captured, ethhdr, ip, UserName);
				DPRINTF("[%d] Client: Response AVAILABLE.\n", (EAP_ID)captured[19]);
				break;
			case MD5:
				DPRINTF("[%d] Server: Request MD5-Challenge!\n", (EAP_ID)captured[19]);
				SendResponseMD5(adhandle, captured, ethhdr, UserName, Password);
				DPRINTF("[%d] Client: Response MD5-Challenge.\n", (EAP_ID)captured[19]);
				break;
			case ALLOCATED: //newtype:7;EAP_REQUEST ALLOCATED PACKAGE;
				DPRINTF("[%d] Server: Request Allocated(Password)!\n", (EAP_ID)captured[19]);
				SendResponsePassword(adhandle, captured, ethhdr, UserName, Password); //Send Password;
				DPRINTF("[%d] Client: Response Allocated(Password).\n", (EAP_ID)captured[19]);
				break;
			case NOTIFICATION:
				DPRINTF("[%d] Server: Request Notification!\n", captured[19]);
				SendResponseNotification(adhandle, captured, ethhdr);
				DPRINTF("     Client: Response Notification.\n");
				break;
			default:
				DPRINTF("[%d] Server: Request (type:%d)!\n", (EAP_ID)captured[19], (EAP_Type)captured[22]);
				DPRINTF("Error! Unexpected request type\n");
				exit(-1);
				break;
			}
		}
		else if ((EAP_Code)captured[18] == FAILURE)
		{ // 处理认证失败信息
			uint8_t errtype = captured[22];
			uint8_t msgsize = captured[23];
			const char *msg = (const char *)&captured[24];
			DPRINTF("[%d] Server: Failure.\n", (EAP_ID)captured[19]);
			if (errtype == 0x09 && msgsize > 0)
			{ // 输出错误提示消息
				fprintf(stderr, "%s\n", msg);
				// 已知的几种错误如下
				// E2531:用户名不存在
				// E2535:Service is paused
				// E2542:该用户帐号已经在别处登录
				// E2547:接入时段限制
				// E2553:密码错误
				// E2602:认证会话不存在
				// E3137:客户端版本号无效

				if (mode == 1)
				{
					goto START_AUTHENTICATION;
				}
				else
				{
					return 0; //exit(-1);
				}
			}
			else if (errtype == 0x08) // 可能网络无流量时朊务器结束此次802.1X认证会话

			{ // 遇此情况客户端立刻发起新的认证会话
				goto START_AUTHENTICATION;
			}
			else
			{
				DPRINTF("errtype=0x%02x\n", errtype);
				exit(-1);
			}
		}
		else if ((EAP_Code)captured[18] == SUCCESS)
		{
			DPRINTF("[%d] Server: Success.\n", captured[19]);
		}
		else if ((EAP_Code)captured[18] == 0x0A)
		{
			DPRINTF("[%d] Server: (H3C data)\n", captured[19]);
			if (captured[26] == 0x35)
			{
				for (i = 0; i < 32; i++)
				{
					AES_MD5req[i] = captured[i + 27];
				}
				h3c_AES_MD5_decryption(AES_MD5data, AES_MD5req);
			}
		}
		else
		{
			DPRINTF("[%d] Server: (unknown data)\n", captured[19]);
		}
	}
}
	return (0);
}

static void GetMacFromDevice(uint8_t mac[6], const char *devicename)
{

	int fd;
	int err;
	struct ifreq ifr;

	fd = socket(PF_PACKET, SOCK_RAW, htons(0x0806));
	assert(fd != -1);

	assert(strlen(devicename) < IFNAMSIZ);
	strncpy(ifr.ifr_name, devicename, IFNAMSIZ);
	ifr.ifr_addr.sa_family = AF_INET;

	err = ioctl(fd, SIOCGIFHWADDR, &ifr);
	assert(err != -1);
	memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);

	err = close(fd);
	assert(err != -1);
	return;
}

static void SendStartPkt(pcap_t *handle, const uint8_t localmac[])
{
	uint8_t packet[18];

	// Ethernet Header (14 Bytes)
	memcpy(packet, BroadcastAddr, 6);
	memcpy(packet + 6, localmac, 6);
	packet[12] = 0x88;
	packet[13] = 0x8e;

	// EAPOL (4 Bytes)
	packet[14] = 0x01;				// Version=1
	packet[15] = 0x01;				// Type=Start
	packet[16] = packet[17] = 0x00; // Length=0x0000

	// 为了兼容不同院校的网络配置，这里发送两遍Start包
	// 1、广播发送Strat包
	pcap_sendpacket(handle, packet, sizeof(packet));
	// 2、多播发送Strat包
	//memcpy(packet, MultcastAddr, 6);
	//pcap_sendpacket(handle, packet, sizeof(packet));
}

static void SendResponseAvailable(pcap_t *handle, const uint8_t request[], const uint8_t ethhdr[], const uint8_t ip[4], const char username[])
{
	int i;
	uint16_t eaplen;
	int usernamelen;
	uint8_t response[128];

	assert((EAP_Code)request[18] == REQUEST);
	assert((EAP_Type)request[22] == AVAILABLE);

	// Fill Ethernet header
	memcpy(response, ethhdr, 14);

	// 802,1X Authentication
	// {
	response[14] = 0x1; // 802.1X Version 1
	response[15] = 0x0; // Type=0 (EAP Packet)
						//response[16~17]留空	// Length

	// Extensible Authentication Protocol
	// {
	response[18] = (EAP_Code)RESPONSE; // Code
	response[19] = request[19];		   // ID
	//response[20~21]留空			// Length
	response[22] = (EAP_Type)AVAILABLE; // Type
										// Type-Data
										// {
	i = 23;
	response[i++] = 0x00;				  // 上报是否使用代理
	response[i++] = 0x15;				  // 上传IP地址
	response[i++] = 0x04;				  //
	memcpy(response + i, ip, 4);		  //
	i += 4;								  //
	response[i++] = 0x06;				  // 携带版本号
	response[i++] = 0x07;				  //
	FillBase64Area((char *)response + i); //
	i += 28;							  //
	response[i++] = ' ';				  // 两个空格符
	response[i++] = ' ';				  //
	usernamelen = strlen(username);
	memcpy(response + i, username, usernamelen); //
	i += usernamelen;							 //
												 // }
												 // }
	// }

	// 补填前面留空的两处Length
	eaplen = htons(i - 18);
	memcpy(response + 16, &eaplen, sizeof(eaplen));
	memcpy(response + 20, &eaplen, sizeof(eaplen));

	// 发送
	pcap_sendpacket(handle, response, i);
}

static void SendResponseIdentity(pcap_t *adhandle, const uint8_t request[], const uint8_t ethhdr[], const uint8_t ip[4], const char username[])
{
	uint8_t response[256];
	size_t i;
	uint16_t eaplen;
	int usernamelen;

	assert((EAP_Code)request[18] == REQUEST);
	assert((EAP_Type)request[22] == IDENTITY || (EAP_Type)request[22] == AVAILABLE); // 兼容中南财经政法大学情况

	// Fill Ethernet header
	memcpy(response, ethhdr, 14);

	// 802,1X Authentication
	// {
	response[14] = 0x1; // 802.1X Version 1
	response[15] = 0x0; // Type=0 (EAP Packet)
						//response[16~17]留空	// Length

	// Extensible Authentication Protocol
	// {
	response[18] = (EAP_Code)RESPONSE; // Code
	response[19] = request[19];		   // ID
	//response[20~21]留空			// Length
	response[22] = (EAP_Type)IDENTITY; // Type
	response[23] = 0x16;
	response[24] = 0x20;
	memcpy(response + 25, AES_MD5data, 32);
	// Type-Data
	// {
	i = 57;
	response[i++] = 0x15;				  // 上传IP地址
	response[i++] = 0x04;				  //
	memcpy(response + i, ip, 4);		  //
	i += 4;								  //
	response[i++] = 0x06;				  // 携带版本号
	response[i++] = 0x07;				  //
	FillBase64Area((char *)response + i); //
	i += 28;							  //
	response[i++] = ' ';				  // 两个空格符
	response[i++] = ' ';				  //
	usernamelen = strlen(username);		  //末尾添加用户名
	memcpy(response + i, username, usernamelen);
	i += usernamelen;
	assert(i <= sizeof(response));
	// }
	// }
	// }

	// 补填前面留空的两处Length
	eaplen = htons(i - 18);
	memcpy(response + 16, &eaplen, sizeof(eaplen));
	memcpy(response + 20, &eaplen, sizeof(eaplen));

	// 发送
	pcap_sendpacket(adhandle, response, i);
	return;
}

void SendFirstResponseIdentity(pcap_t *adhandle, const uint8_t request[], const uint8_t ethhdr[], const uint8_t ip[4], const char username[])
{
	uint8_t response[128];
	size_t i;
	uint16_t eaplen;
	int usernamelen;

	assert((EAP_Code)request[18] == REQUEST);
	assert((EAP_Type)request[22] == IDENTITY || (EAP_Type)request[22] == AVAILABLE); // 兼容中南财经政法大学情况

	// Fill Ethernet header
	memcpy(response, ethhdr, 14);

	// 802,1X Authentication
	// {
	response[14] = 0x1; // 802.1X Version 1
	response[15] = 0x0; // Type=0 (EAP Packet)
						//response[16~17]留空	// Length

	// Extensible Authentication Protocol
	// {
	response[18] = (EAP_Code)RESPONSE; // Code
	response[19] = request[19];		   // ID
	//response[20~21]留空			// Length
	response[22] = (EAP_Type)IDENTITY; // Type
									   // Type-Data
									   // {
	i = 23;
	//response[i++] = 0x15;	  // 上传IP地址
	//response[i++] = 0x04;	  //
	//memcpy(response+i, ip, 4);//
	//i += 4;			  //
	response[i++] = 0x06;				  // 携带版本号
	response[i++] = 0x07;				  //
	FillBase64Area((char *)response + i); //
	i += 28;							  //
	response[i++] = ' ';				  // 两个空格符
	response[i++] = ' ';				  //
	usernamelen = strlen(username);		  //末尾添加用户名
	memcpy(response + i, username, usernamelen);
	i += usernamelen;
	assert(i <= sizeof(response));
	// }
	// }
	// }

	// 补填前面留空的两处Length
	eaplen = htons(i - 18);
	memcpy(response + 16, &eaplen, sizeof(eaplen));
	memcpy(response + 20, &eaplen, sizeof(eaplen));

	// 发送
	pcap_sendpacket(adhandle, response, i);
	return;
}

static void SendResponseMD5(pcap_t *handle, const uint8_t request[], const uint8_t ethhdr[], const char username[], const char passwd[])
{
	uint16_t eaplen;
	size_t usernamelen;
	size_t packetlen;
	uint8_t response[128];

	assert((EAP_Code)request[18] == REQUEST);
	assert((EAP_Type)request[22] == MD5);

	usernamelen = strlen(username);
	eaplen = htons(22 + usernamelen);
	packetlen = 14 + 4 + 22 + usernamelen; // ethhdr+EAPOL+EAP+usernamelen

	// Fill Ethernet header
	memcpy(response, ethhdr, 14);

	// 802,1X Authentication
	// {
	response[14] = 0x1;								// 802.1X Version 1
	response[15] = 0x0;								// Type=0 (EAP Packet)
	memcpy(response + 16, &eaplen, sizeof(eaplen)); // Length

	// Extensible Authentication Protocol
	// {
	response[18] = (EAP_Code)RESPONSE; // Code
	response[19] = request[19];		   // ID
	response[20] = response[16];	   // Length
	response[21] = response[17];	   //
	response[22] = (EAP_Type)MD5;	  // Type
	response[23] = 16;				   // Value-Size: 16 Bytes
	FillMD5Area(response + 24, request[19], passwd, request + 24);
	memcpy(response + 40, username, usernamelen);
	// }
	// }

	pcap_sendpacket(handle, response, packetlen);
}

static void SendResponsePassword(pcap_t *handle, const uint8_t request[], const uint8_t ethhdr[], const char username[], const char passwd[]) //SendResponsePassword(adhandle, captured, ethhdr, UserName, Password);
{
	uint16_t eaplen;
	size_t usernamelen;
	size_t passwordlen;
	size_t packetlen;
	uint8_t response[128];

	assert((EAP_Code)request[18] == REQUEST);
	assert((EAP_Type)request[22] == ALLOCATED);

	usernamelen = strlen(username);
	passwordlen = strlen(passwd);
	eaplen = htons(6 + usernamelen + passwordlen);
	packetlen = 24 + usernamelen + passwordlen; //14+4+22+usernamelen; // ethhdr+EAPOL+EAP+usernamelen

	// Fill Ethernet header
	memcpy(response, ethhdr, 14);

	// 802.1X Authentication
	// {
	response[14] = 0x1;								// 802.1X Version 1
	response[15] = 0x0;								// Type=0 (EAP Packet)
	memcpy(response + 16, &eaplen, sizeof(eaplen)); // Length

	// Extensible Authentication Protocol
	// {
	response[18] = (EAP_Code)RESPONSE;  // Code
	response[19] = request[19];			// ID
	response[20] = response[16];		// Length
	response[21] = response[17];		//
	response[22] = (EAP_Type)ALLOCATED; // Type
	response[23] = passwordlen;			// Value-Size: 16 Bytes
	memcpy(response + 24, passwd, passwordlen);
	memcpy(response + 24 + passwordlen + 1, username, usernamelen); //?????
	// }

	pcap_sendpacket(handle, response, packetlen);
}

static void SendLogoffPkt(pcap_t *handle, const uint8_t localmac[])
{
	uint8_t packet[18];

	// Ethernet Header (14 Bytes)
	memcpy(packet, MultcastAddr, 6);
	memcpy(packet + 6, localmac, 6);
	packet[12] = 0x88;
	packet[13] = 0x8e;

	// EAPOL (4 Bytes)
	packet[14] = 0x01;				// Version=1
	packet[15] = 0x02;				// Type=Logoff
	packet[16] = packet[17] = 0x00; // Length=0x0000

	// 发包
	pcap_sendpacket(handle, packet, sizeof(packet));
}

// 函数: XOR(data[], datalen, key[], keylen)
//
// 使用密钥key[]对数据data[]进行异或加密
//（注：该函数也可反向用于解密）
static void XOR(uint8_t data[], unsigned dlen, const char key[], unsigned klen)
{
	unsigned int i, j;

	// 先按正序处理一遍
	for (i = 0; i < dlen; i++)
		data[i] ^= key[i % klen];
	// 再按倒序处理第二遍
	for (i = dlen - 1, j = 0; j < dlen; i--, j++)
		data[i] ^= key[j % klen];
}

static void FillClientVersionArea(uint8_t area[20])
{
	uint32_t random;
	char RandomKey[8 + 1];

	random = (uint32_t)time(NULL);		// 注：可以选任意32位整数
	sprintf(RandomKey, "%08x", random); // 生成RandomKey[]字符串

	// 第一轮异或运算，以RandomKey为密钥加密16字节
	memcpy(area, H3C_VERSION, sizeof(H3C_VERSION));
	XOR(area, 16, RandomKey, strlen(RandomKey));

	// 此16字节加上4字节的random，组成总计20字节
	random = htonl(random); // （需调整为网络字节序）
	memcpy(area + 16, &random, 4);

	// 第二轮异或运算，以H3C_KEY为密钥加密前面生成的20字节
	XOR(area, 20, H3C_KEY, strlen(H3C_KEY));
}

static void FillWindowsVersionArea(uint8_t area[20])
{
	const uint8_t WinVersion[20] = "r70393861";

	memcpy(area, WinVersion, 20);
	XOR(area, 20, H3C_KEY, strlen(H3C_KEY));
}

static void SendResponseNotification(pcap_t *handle, const uint8_t request[], const uint8_t ethhdr[])
{
	uint8_t response[67];

	assert((EAP_Code)request[18] == REQUEST);
	assert((EAP_Type)request[22] == NOTIFICATION);

	// Fill Ethernet header
	memcpy(response, ethhdr, 14);

	// 802,1X Authentication
	// {
	response[14] = 0x1;  // 802.1X Version 1
	response[15] = 0x0;  // Type=0 (EAP Packet)
	response[16] = 0x00; // Length
	response[17] = 0x31; //

	// Extensible Authentication Protocol
	// {
	response[18] = (EAP_Code)RESPONSE;	 // Code
	response[19] = (EAP_ID)request[19];	// ID
	response[20] = response[16];		   // Length
	response[21] = response[17];		   //
	response[22] = (EAP_Type)NOTIFICATION; // Type

	int i = 23;
	/* Notification Data (44 Bytes) */
	// 其中前2+20字节为客户端版本
	response[i++] = 0x01; // type 0x01
	response[i++] = 22;   // lenth
	FillClientVersionArea(response + i);
	i += 20;

	// 最后2+20字节存储加密后的Windows操作系统版本号
	response[i++] = 0x02; // type 0x02
	response[i++] = 22;   // length
	FillWindowsVersionArea(response + i);
	i += 20;
	// }
	// }

	pcap_sendpacket(handle, response, sizeof(response));
}

static void FillBase64Area(char area[])
{
	uint8_t version[20];
	const char Tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					   "abcdefghijklmnopqrstuvwxyz"
					   "0123456789+/"; // 标准的Base64字符映射表
	uint8_t c1, c2, c3;
	int i, j;

	// 首先生成20字节加密过的H3C版本号信息
	FillClientVersionArea(version);

	// 然后按照Base64编码法将前面生成的20字节数据转换为28字节ASCII字符
	i = 0;
	j = 0;
	while (j < 24)
	{
		c1 = version[i++];
		c2 = version[i++];
		c3 = version[i++];
		area[j++] = Tbl[(c1 & 0xfc) >> 2];
		area[j++] = Tbl[((c1 & 0x03) << 4) | ((c2 & 0xf0) >> 4)];
		area[j++] = Tbl[((c2 & 0x0f) << 2) | ((c3 & 0xc0) >> 6)];
		area[j++] = Tbl[c3 & 0x3f];
	}
	c1 = version[i++];
	c2 = version[i++];
	area[24] = Tbl[(c1 & 0xfc) >> 2];
	area[25] = Tbl[((c1 & 0x03) << 4) | ((c2 & 0xf0) >> 4)];
	area[26] = Tbl[((c2 & 0x0f) << 2)];
	area[27] = '=';
}
