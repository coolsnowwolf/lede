/* File: main.c
 * ------------
 * 校园网802.1X客户端命令行
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

/* 子函数声明 */
int Authentication(const char *UserName, const char *Password, const char *DeviceName, const char *Version, const char *Key);
void convert(char *dest, char const *src);

/**
 * 函数：main()
 *
 * 检查程序的执行权限，检查命令行参数格式。
 * 允许的调用格式包括：
 * 	njit-client  username  password
 * 	njit-client  username  password  eth0
 * 	njit-client  username  password  eth1
 * 若没有从命令行指定网卡，则默认将使用eth0
 */
int main(int argc, char *argv[])
{
	char *UserName;
	char *Password;
	char *DeviceName;
	char *Version;
	char *Key;

	/* 检查当前是否具有root权限 */
	if (getuid() != 0) {
		fprintf(stderr, "抱歉，运行本客户端程序需要root权限\n");
		fprintf(stderr, "(RedHat/Fedora下使用su命令切换为root)\n");
		fprintf(stderr, "(Ubuntu/Debian下在命令前添加sudo)\n");
		exit(-1);
	}

	/* 检查命令行参数格式 */
	if (argc<3 || argc>6) {
		fprintf(stderr, "命令行参数错误！\n");
		fprintf(stderr,	"正确的调用格式例子如下：\n");
		fprintf(stderr,	"    %s username password\n", argv[0]);
		fprintf(stderr,	"    %s username password eth0\n", argv[0]);
		fprintf(stderr,	"    %s username password eth0 version key\n", argv[0]);
		fprintf(stderr, "(注：若不指明网卡，默认情况下将使用eth0.2)\n");
		exit(-1);
	} else {
		if (argc > 3) {
			DeviceName = argv[3]; // 允许从命令行指定设备名
		} else {
			DeviceName = "eth0.2"; // 缺省情况下使用的设备
		}
		if (argc > 5) {
			Version = (char *)malloc(32 * sizeof(char));
			convert(Version, argv[4]);
			Key = argv[5];
		} else {
			Version = "CH\x11V7.10-0313";
			Key = "Oly5D62FaE94W7";
		}
	}
	UserName = argv[1];
	Password = argv[2];

	/* 调用子函数完成802.1X认证 */
	Authentication(UserName, Password, DeviceName, Version, Key);

	return (0);
}


void convert(char *dest, char const *src)
{
	int i = 0, j = 0;
	int len = strlen(src);
	while (i < len) {
		if (src[i] == '\\' && i+3 < len && src[i+1] == 'x') {
			char s[3];
			s[0] = src[i+2];
			s[1] = src[i+3];
			s[2] = 0;
			char c;
			sscanf(s, "%x", &c);
			dest[j++] = c;
			i += 4;
		} else {
			dest[j++] = src[i++];
		}
	}
	dest[j] = 0;
}