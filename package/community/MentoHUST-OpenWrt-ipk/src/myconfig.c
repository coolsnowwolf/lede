/* -*- Mode: C; tab-width: 4; -*- */
/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：myconfig.c
* 摘	要：初始化认证参数
* 作	者：HustMoon@BYHH
* 邮	箱：www.ehust@gmail.com
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#else
static const char *VERSION = "0.3.1";
static const char *PACKAGE_BUGREPORT = "http://code.google.com/p/mentohust/issues/list";
#endif

#include "myconfig.h"
#include "myini.h"
#include "myfunc.h"
#include "dlfunc.h"
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

#define ACCOUNT_SIZE		65	/* 用户名密码长度*/
#define NIC_SIZE			16	/* 网卡名最大长度 */
#define MAX_PATH			255	/* FILENAME_MAX */
#define D_TIMEOUT			8	/* 默认超时间隔 */
#define D_ECHOINTERVAL		30	/* 默认心跳间隔 */
#define D_RESTARTWAIT		15	/* 默认重连间隔 */
#define D_STARTMODE			0	/* 默认组播模式 */
#define D_DHCPMODE			0	/* 默认DHCP模式 */
#define D_DAEMONMODE		0	/* 默认daemon模式 */
#define D_MAXFAIL			0	/* 默认允许失败次数 */

static const char *D_DHCPSCRIPT = "dhclient";	/* 默认DHCP脚本 */
static const char *CFG_FILE = "/etc/mentohust.conf";	/* 配置文件 */
static const char *LOG_FILE = "/tmp/mentohust.log";	/* 日志文件 */
static const char *LOCK_FILE = "/tmp/mentohust.pid";	/* 锁文件 */
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)	/* 创建掩码 */

#ifndef NO_NOTIFY
#define D_SHOWNOTIFY		5	/* 默认Show Notify模式 */
int showNotify = D_SHOWNOTIFY;	/* 显示通知 */
#endif

extern int bufType;	/*0内置xrgsu 1内置Win 2仅文件 3文件+校验*/
extern u_char version[];	/* 版本 */
char userName[ACCOUNT_SIZE] = "";	/* 用户名 */
char password[ACCOUNT_SIZE] = "";	/* 密码 */
char nic[NIC_SIZE] = "";	/* 网卡名 */
char dataFile[MAX_PATH] = "";	/* 数据文件 */
char dhcpScript[MAX_PATH] = "";	/* DHCP脚本 */
u_int32_t ip = 0;	/* 本机IP */
u_int32_t mask = 0;	/* 子网掩码 */
u_int32_t gateway = 0;	/* 网关 */
u_int32_t dns = 0;	/* DNS */
u_int32_t pingHost = 0;	/* ping */
u_char localMAC[6];	/* 本机MAC */
u_char destMAC[6];	/* 服务器MAC */
unsigned timeout = D_TIMEOUT;	/* 超时间隔 */
unsigned echoInterval = D_ECHOINTERVAL;	/* 心跳间隔 */
unsigned restartWait = D_RESTARTWAIT;	/* 失败等待 */
unsigned startMode = D_STARTMODE;	/* 组播模式 */
unsigned dhcpMode = D_DHCPMODE;	/* DHCP模式 */
unsigned maxFail = D_MAXFAIL;	/* 允许失败次数 */
pcap_t *hPcap = NULL;	/* Pcap句柄 */
int lockfd = -1;	/* 锁文件描述符 */

static int readFile(int *daemonMode);	/* 读取配置文件来初始化 */
static void readArg(char argc, char **argv, int *saveFlag, int *exitFlag, int *daemonMode);	/* 读取命令行参数来初始化 */
static void showHelp(const char *fileName);	/* 显示帮助信息 */
static int getAdapter();	/* 查找网卡名 */
static void printConfig();	/* 显示初始化后的认证参数 */
static int openPcap();	/* 初始化pcap、设置过滤器 */
static void saveConfig(int daemonMode);	/* 保存参数 */
static void checkRunning(int exitFlag, int daemonMode);	/* 检测是否已运行 */

#ifndef NO_ENCODE_PASS
static const unsigned char base64Tab[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
static const char xorRuijie[] = {"~!:?$*<(qw2e5o7i8x12c6m67s98w43d2l45we82q3iuu1z4xle23rt4oxclle34e54u6r8m"};

static int encodePass(char *dst, const char *osrc) {
    unsigned char in[3], buf[70];
	unsigned char *src = buf;
	int sz = strlen(osrc);
    int i, len;
	if (sizeof(xorRuijie) < sz)
		return -1;
	for(i=0; i<sz; i++)
		src[i] = osrc[i] ^ xorRuijie[i];
    while (sz > 0) {
        for (len=0, i=0; i<3; i++, sz--) {
			if (sz > 0) {
				len++;
				in[i] = src[i];
            } else in[i] = 0;
        }
        src += 3;
        if (len) {
			dst[0] = base64Tab[ in[0] >> 2 ];
			dst[1] = base64Tab[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
			dst[2] = len > 1 ? base64Tab[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=';
			dst[3] = len > 2 ? base64Tab[ in[2] & 0x3f ] : '=';
            dst += 4;
        }
    }
    *dst = '\0';
	return 0;
}

static int decodePass(char *dst, const char *src) {
	unsigned esi = 0, idx = 0;
	int i=0, j=0, equal=0;
	for(; src[i]!='\0'; i++) {
		if (src[i] == '=') {
			if (++equal > 2)
				return -1;
		} else {
			for(idx=0; base64Tab[idx]!='\0'; idx++) {
				if(base64Tab[idx] == src[i])
					break;
			}
			if (idx == 64)
				return -1;
			esi += idx;
		}
		if(i%4 == 3) {
			dst[j++] = (char)(esi>>16);
			if(equal < 2)
				dst[j++] = (char)(esi>>8);
			if(equal < 1)
				dst[j++] = (char)esi;
			esi = 0;
			equal = 0;
		}
		esi <<= 6;
	}
	if (i%4!=0 || sizeof(xorRuijie)<j)
		return -1;
	for(i=0; i<j; i++)
		dst[i] ^= xorRuijie[i];
	dst[j] = '\0';
	return 0;
}
#endif

void initConfig(int argc, char **argv)
{
	int saveFlag = 0;	/* 是否需要保存参数 */
	int exitFlag = 0;	/* 0Nothing 1退出 2重启 */
	int daemonMode = D_DAEMONMODE;	/* 是否后台运行 */

	printf("\n欢迎使用MentoHUST\t版本: %s\n"
			"Copyright (C) 2009-2010 HustMoon Studio\n"
			"人到华中大，有甜亦有辣。明德厚学地，求是创新家。\n"
			"Bug report to %s\n\n", VERSION, PACKAGE_BUGREPORT);
	saveFlag = (readFile(&daemonMode)==0 ? 0 : 1);
	readArg(argc, argv, &saveFlag, &exitFlag, &daemonMode);
#ifndef NO_NOTIFY
	if (showNotify) {
		seteuid(getuid());
		if (load_libnotify() == -1)
			showNotify = 0;
		else
			set_timeout(1000 * showNotify);
		seteuid(0);
	}
#endif
#ifndef NO_DYLOAD
	if (load_libpcap() == -1) {
#ifndef NO_NOTIFY
		if (showNotify)
			show_notify("MentoHUST - 错误提示", "载入libpcap失败, 请检查该库文件！");
#endif
		exit(EXIT_FAILURE);
	}
#endif
	if (nic[0] == '\0')
	{
		saveFlag = 1;
		if (getAdapter() == -1) {	/* 找不到（第一块）网卡？ */
#ifndef NO_NOTIFY
			if (showNotify)
				show_notify("MentoHUST - 错误提示", "找不到网卡！");
#endif
			exit(EXIT_FAILURE);
		}
	}
	if (userName[0]=='\0' || password[0]=='\0')	/* 未写用户名密码？ */
	{
		saveFlag = 1;
		printf("?? 请输入用户名: ");
		scanf("%s", userName);
		printf("?? 请输入密码: ");
		scanf("%s", password);
		printf("?? 请选择组播地址(0标准 1锐捷私有 2赛尔): ");
		scanf("%u", &startMode);
		startMode %= 3;
		printf("?? 请选择DHCP方式(0不使用 1二次认证 2认证后 3认证前): ");
		scanf("%u", &dhcpMode);
		dhcpMode %= 4;
	}
	checkRunning(exitFlag, daemonMode);
	if (startMode%3==2 && gateway==0)	/* 赛尔且未填写网关地址 */
	{
		gateway = ip;	/* 据说赛尔的网关是ip前三字节，后一字节是2 */
		((u_char *)&gateway)[3] = 0x02;
	}
	if (dhcpScript[0] == '\0')	/* 未填写DHCP脚本？ */
		strcpy(dhcpScript, D_DHCPSCRIPT);
	newBuffer();
	printConfig();
	if (fillHeader()==-1 || openPcap()==-1) {	/* 获取IP、MAC，打开网卡 */
#ifndef NO_NOTIFY
		if (showNotify)
			show_notify("MentoHUST - 错误提示", "获取MAC地址或打开网卡失败！");
#endif
		exit(EXIT_FAILURE);
	}
	if (saveFlag)
		saveConfig(daemonMode);
}

static int readFile(int *daemonMode)
{
	char tmp[16];
	char *buf = loadFile(CFG_FILE);
	if (buf == NULL)
		return -1;
	getString(buf, "MentoHUST", "Username", "", userName, sizeof(userName));
	getString(buf, "MentoHUST", "Password", "", password, sizeof(password));
#ifndef NO_ENCODE_PASS
	char pass[ACCOUNT_SIZE*4/3];
	if (password[0] == '\0') {
		getString(buf, "MentoHUST", "EncodePass", "", pass, sizeof(pass));
		decodePass(password, pass);
	} else {
		encodePass(pass, password);
		setString(&buf, "MentoHUST", "Password", NULL);
		setString(&buf, "MentoHUST", "EncodePass", pass);
		saveFile(buf, CFG_FILE);
	}
#endif
	getString(buf, "MentoHUST", "Nic", "", nic, sizeof(nic));
	getString(buf, "MentoHUST", "Datafile", "", dataFile, sizeof(dataFile));
	getString(buf, "MentoHUST", "DhcpScript", "", dhcpScript, sizeof(dhcpScript));
	getString(buf, "MentoHUST", "Version", "", tmp, sizeof(tmp));
	if (strlen(tmp) >= 3) {
		unsigned ver[2];
		if (sscanf(tmp, "%u.%u", ver, ver+1)!=EOF && ver[0]!=0) {
			version[0] = ver[0];
			version[1] = ver[1];
			bufType = 1;
		}
	}
	getString(buf, "MentoHUST", "IP", "255.255.255.255", tmp, sizeof(tmp));
	ip = inet_addr(tmp);
	getString(buf, "MentoHUST", "Mask", "255.255.255.255", tmp, sizeof(tmp));
	mask = inet_addr(tmp);
	getString(buf, "MentoHUST", "Gateway", "0.0.0.0", tmp, sizeof(tmp));
	gateway = inet_addr(tmp);
	getString(buf, "MentoHUST", "DNS", "0.0.0.0", tmp, sizeof(tmp));
	dns = inet_addr(tmp);
	getString(buf, "MentoHUST", "PingHost", "0.0.0.0", tmp, sizeof(tmp));
	pingHost = inet_addr(tmp);
	timeout = getInt(buf, "MentoHUST", "Timeout", D_TIMEOUT) % 100;
	echoInterval = getInt(buf, "MentoHUST", "EchoInterval", D_ECHOINTERVAL) % 1000;
	restartWait = getInt(buf, "MentoHUST", "RestartWait", D_RESTARTWAIT) % 100;
	startMode = getInt(buf, "MentoHUST", "StartMode", D_STARTMODE) % 3;
	dhcpMode = getInt(buf, "MentoHUST", "DhcpMode", D_DHCPMODE) % 4;
#ifndef NO_NOTIFY
	showNotify = getInt(buf, "MentoHUST", "ShowNotify", D_SHOWNOTIFY) % 21;
#endif
	*daemonMode = getInt(buf, "MentoHUST", "DaemonMode", D_DAEMONMODE) % 4;
	maxFail = getInt(buf, "MentoHUST", "MaxFail", D_MAXFAIL);
	free(buf);
	return 0;
}

static void readArg(char argc, char **argv, int *saveFlag, int *exitFlag, int *daemonMode)
{
	char *str, c;
	int i;
	for (i=1; i<argc; i++)
	{
		str = argv[i];
		if (str[0]!='-' && str[0]!='/')
			continue;
		c = str[1];
		if (c=='h' || c=='?' || strcmp(str, "--help")==0)
			showHelp(argv[0]);
		else if (c == 'w')
			*saveFlag = 1;
		else if (c == 'k') {
			if (strlen(str) > 2)
				*exitFlag = 2;
			else {
				*exitFlag = 1;
				return;
			}
		} else if (strlen(str) > 2) {
			if (c == 'u')
				strncpy(userName, str+2, sizeof(userName)-1);
			else if (c == 'p')
				strncpy(password, str+2, sizeof(password)-1);
			else if (c == 'n')
				strncpy(nic, str+2, sizeof(nic)-1);
			else if (c == 'f')
				strncpy(dataFile, str+2, sizeof(dataFile)-1);
			else if (c == 'c')
				strncpy(dhcpScript, str+2, sizeof(dhcpScript)-1);
			else if (c=='v' && strlen(str+2)>=3) {
				unsigned ver[2];
				if (sscanf(str+2, "%u.%u", ver, ver+1) != EOF) {
					if (ver[0] == 0)
						bufType = 0;
					else {
						version[0] = ver[0];
						version[1] = ver[1];
						bufType = 1;
					}
				}
			}
			else if (c == 'i')
				ip = inet_addr(str+2);
			else if (c == 'm')
				mask = inet_addr(str+2);
			else if (c == 'g')
				gateway = inet_addr(str+2);
			else if (c == 's')
				dns = inet_addr(str+2);
			else if (c == 'o')
				pingHost = inet_addr(str+2);
			else if (c == 't')
				timeout = atoi(str+2) % 100;
			else if (c == 'e')
				echoInterval = atoi(str+2) % 1000;
			else if (c == 'r')
				restartWait = atoi(str+2) % 100;
			else if (c == 'a')
				startMode = atoi(str+2) % 3;
			else if (c == 'd')
				dhcpMode = atoi(str+2) % 4;
#ifndef NO_NOTIFY
			else if (c == 'y')
				showNotify = atoi(str+2) % 21;
#endif
			else if (c == 'b')
				*daemonMode = atoi(str+2) % 4;
			else if (c == 'l')
				maxFail = atoi(str+2);
		}
	}
}

static void showHelp(const char *fileName)
{
	char *helpString =
		"用法:\t%s [-选项][参数]\n"
		"选项:\t-h 显示本帮助信息\n"
		"\t-k -k(退出程序) 其他(重启程序)\n"
		"\t-w 保存参数到配置文件\n"
		"\t-u 用户名\n"
		"\t-p 密码\n"
		"\t-n 网卡名\n"
		"\t-i IP[默认本机IP]\n"
		"\t-m 子网掩码[默认本机掩码]\n"
		"\t-g 网关[默认0.0.0.0]\n"
		"\t-s DNS[默认0.0.0.0]\n"
		"\t-o Ping主机[默认0.0.0.0，表示关闭该功能]\n"
		"\t-t 认证超时(秒)[默认8]\n"
		"\t-e 响应间隔(秒)[默认30]\n"
		"\t-r 失败等待(秒)[默认15]\n"
		"\t-l 允许失败次数[默认0，表示无限制]\n"
		"\t-a 组播地址: 0(标准) 1(锐捷) 2(赛尔) [默认0]\n"
		"\t-d DHCP方式: 0(不使用) 1(二次认证) 2(认证后) 3(认证前) [默认0]\n"
		"\t-b 是否后台运行: 0(否) 1(是，关闭输出) 2(是，保留输出) 3(是，输出到文件) [默认0]\n"
#ifndef NO_NOTIFY
		"\t-y 是否显示通知: 0(否) 1~20(是) [默认5]\n"
#endif
		"\t-v 客户端版本号[默认0.00表示兼容xrgsu]\n"
		"\t-f 自定义数据文件[默认不使用]\n"
		"\t-c DHCP脚本[默认dhclient]\n"
		"例如:\t%s -uusername -ppassword -neth0 -i192.168.0.1 -m255.255.255.0 -g0.0.0.0 -s0.0.0.0 -o0.0.0.0 -t8 -e30 -r15 -a0 -d1 -b0 -v4.10 -fdefault.mpf -cdhclient\n"
		"注意：使用时请确保是以root权限运行！\n\n";
	printf(helpString, fileName, fileName);
	exit(EXIT_SUCCESS);
}

static int getAdapter()
{
	pcap_if_t *alldevs, *d;
	int num = 0, avail = 0, i;
	char errbuf[PCAP_ERRBUF_SIZE];
	if (pcap_findalldevs(&alldevs, errbuf)==-1 || alldevs==NULL)
	{
		printf("!! 查找网卡失败: %s\n", errbuf);
		return -1;
	}
	for (d=alldevs; d!=NULL; d=d->next)
	{
		num++;
		if (!(d->flags & PCAP_IF_LOOPBACK) && strcmp(d->name, "any")!=0)
		{
			printf("** 网卡[%d]:\t%s\n", num, d->name);
			avail++;
			i = num;
		}
	}
	if (avail == 0)
	{
		pcap_freealldevs(alldevs);
		printf("!! 找不到网卡！\n");
		return -1;
	}
	if (avail > 1)
	{
		printf("?? 请选择网卡[1-%d]: ", num);
		scanf("%d", &i);
		if (i < 1)
			i = 1;
		else if (i > num)
			i = num;
	}
	printf("** 您选择了第[%d]块网卡。\n", i);
	for (d=alldevs; i>1; d=d->next, i--);
	strncpy(nic, d->name, sizeof(nic)-1);
	pcap_freealldevs(alldevs);
	return 0;
}

static void printConfig()
{
	char *addr[] = {"标准", "锐捷", "赛尔"};
	char *dhcp[] = {"不使用", "二次认证", "认证后", "认证前"};
	printf("** 用户名:\t%s\n", userName);
	/* printf("** 密码:\t%s\n", password); */
	printf("** 网卡: \t%s\n", nic);
	if (gateway)
		printf("** 网关地址:\t%s\n", formatIP(gateway));
	if (dns)
		printf("** DNS地址:\t%s\n", formatIP(dns));
	if (pingHost)
		printf("** 智能重连:\t%s\n", formatIP(pingHost));
	printf("** 认证超时:\t%u秒\n", timeout);
	printf("** 响应间隔:\t%u秒\n", echoInterval);
	printf("** 失败等待:\t%u秒\n", restartWait);
	printf("** 允许失败:\t%u次\n", maxFail);
	printf("** 组播地址:\t%s\n", addr[startMode]);
	printf("** DHCP方式:\t%s\n", dhcp[dhcpMode]);
#ifndef NO_NOTIFY
	if (showNotify)
		printf("** 通知超时:\t%d秒\n", showNotify);
#endif
	if (bufType >= 2)
		printf("** 数据文件:\t%s\n", dataFile);
	if (dhcpMode != 0)
		printf("** DHCP脚本:\t%s\n", dhcpScript);
}

static int openPcap()
{
	char buf[PCAP_ERRBUF_SIZE], *fmt;
	struct bpf_program fcode;
	if ((hPcap = pcap_open_live(nic, 2048, 1, 1000, buf)) == NULL)
	{
		printf("!! 打开网卡%s失败: %s\n", nic, buf);
		return -1;
	}
	fmt = formatHex(localMAC, 6);
#ifndef NO_ARP
	sprintf(buf, "((ether proto 0x888e and (ether dst %s or ether dst 01:80:c2:00:00:03)) "
			"or ether proto 0x0806) and not ether src %s", fmt, fmt);
#else
	sprintf(buf, "ether proto 0x888e and (ether dst %s or ether dst 01:80:c2:00:00:03) "
			"and not ether src %s", fmt, fmt);
#endif
	if (pcap_compile(hPcap, &fcode, buf, 0, 0xffffffff) == -1
			|| pcap_setfilter(hPcap, &fcode) == -1)
	{
		printf("!! 设置pcap过滤器失败: %s\n", pcap_geterr(hPcap));
		return -1;
	}
	pcap_freecode(&fcode);
	return 0;
}

static void saveConfig(int daemonMode)
{
	char *buf = loadFile(CFG_FILE);
	if (buf == NULL) {
		buf = (char *)malloc(1);
		buf[0] = '\0';
	}
	setString(&buf, "MentoHUST", "DhcpScript", dhcpScript);
	setString(&buf, "MentoHUST", "DataFile", dataFile);
	if (bufType != 0) {
		char ver[10];
		sprintf(ver, "%u.%u", version[0], version[1]);
		setString(&buf, "MentoHUST", "Version", ver);
	} else
		setString(&buf, "MentoHUST", "Version", "0.00");
#ifndef NO_NOTIFY
	setInt(&buf, "MentoHUST", "ShowNotify", showNotify);
#endif
	setInt(&buf, "MentoHUST", "DaemonMode", daemonMode);
	setInt(&buf, "MentoHUST", "DhcpMode", dhcpMode);
	setInt(&buf, "MentoHUST", "StartMode", startMode);
	setInt(&buf, "MentoHUST", "MaxFail", maxFail);
	setInt(&buf, "MentoHUST", "RestartWait", restartWait);
	setInt(&buf, "MentoHUST", "EchoInterval", echoInterval);
	setInt(&buf, "MentoHUST", "Timeout", timeout);
	setString(&buf, "MentoHUST", "PingHost", formatIP(pingHost));
	setString(&buf, "MentoHUST", "DNS", formatIP(dns));
	setString(&buf, "MentoHUST", "Gateway", formatIP(gateway));
	setString(&buf, "MentoHUST", "Mask", formatIP(mask));
	setString(&buf, "MentoHUST", "IP", formatIP(ip));
	setString(&buf, "MentoHUST", "Nic", nic);
#ifdef NO_ENCODE_PASS
	setString(&buf, "MentoHUST", "Password", password);
#else
	char pass[ACCOUNT_SIZE*4/3];
	encodePass(pass, password);
	setString(&buf, "MentoHUST", "EncodePass", pass);
#endif
	setString(&buf, "MentoHUST", "Username", userName);
	if (saveFile(buf, CFG_FILE) != 0)
		printf("!! 保存认证参数到%s失败！\n", CFG_FILE);
	else
		printf("** 认证参数已成功保存到%s.\n", CFG_FILE);
	free(buf);
}

static void checkRunning(int exitFlag, int daemonMode)
{
	struct flock fl;
	lockfd = open (LOCK_FILE, O_RDWR|O_CREAT, LOCKMODE);
	if (lockfd < 0) {
		perror("!! 打开锁文件失败");	/* perror真的很好啊，以前没用它真是太亏了 */
		goto error_exit;
	}
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	fl.l_type = F_WRLCK;
	if (fcntl(lockfd, F_GETLK, &fl) < 0) {
		perror("!! 获取文件锁失败");
		goto error_exit;
	}
	if (exitFlag) {
		if (fl.l_type != F_UNLCK) {
			printf(">> 已发送退出信号给MentoHUST进程(PID=%d).\n", fl.l_pid);
			if (kill(fl.l_pid, SIGINT) == -1)
				perror("!! 结束进程失败");
		}
		else
			printf("!! 没有MentoHUST正在运行！\n");
		if (exitFlag == 1)
			exit(EXIT_SUCCESS);
	}
	else if (fl.l_type != F_UNLCK) {
		printf("!! MentoHUST已经运行(PID=%d)!\n", fl.l_pid);
		exit(EXIT_FAILURE);
	}
	if (daemonMode) {	/* 貌似我过早进入后台模式了，就给个选项保留输出或者输出到文件吧 */
		printf(">> 进入后台运行模式，使用参数-k可退出认证。\n");
		if (daemon(0, (daemonMode+1)%2))
			perror("!! 后台运行失败");
		else if (daemonMode == 3) {
			freopen(LOG_FILE, "w", stdout);
			freopen(LOG_FILE, "a", stderr);
		}
	}
	fl.l_type = F_WRLCK;
	fl.l_pid = getpid();
	if (fcntl(lockfd, F_SETLKW, &fl) < 0) {
		perror("!! 加锁失败");
		goto error_exit;
	}
	return;

error_exit:
#ifndef NO_NOTIFY
	if (showNotify)
		show_notify("MentoHUST - 错误提示", "操作锁文件失败，请检查是否为root权限！");
#endif
	exit(EXIT_FAILURE);
}
