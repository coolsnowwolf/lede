#ifndef ADAPTER_H_
#define ADAPTER_H_

#include <cstdint>

#include "global.h"

// 获取正确的设备名
bool GetAdapterName(pcap_if_t *device, char*adapterName);

// 通过设备名获取Mac地址，mac变量应事先分配6unsigned chars以上
bool GetMacAddress(const char* adapterName, unsigned char* mac);

// 通过网卡名获取ip，ip变量应事先分配17unsigned chars以上
bool GetIp(const char * adapterName, unsigned char * ip);

// 装换成pcap识别的设备名
bool AdapterNameToPcap(const char * adapterName, char * pcapAdapterName);

int GetAdapter(char ** adapterName);

// style为0时不做任何工作，为1时使用dhclient, 为2时使用udhcpc
bool DhcpClient(const char *adapterName, int style = 0, const char *dhcpScript = NULL);

// 一些调试用的函数
/*
// C prototype : void StrToHex(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 输出缓冲区
//	[IN] pbSrc - 字符串
//	[IN] nLen - 16进制数的字节数(字符串的长度/2)
// return value:
// remarks : 将字符串转化为16进制数
*/
void StrToHex(unsigned char *pbDest, unsigned char *pbSrc, int nLen);

/*
// C prototype : void HexToStr(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 存放目标字符串
//	[IN] pbSrc - 输入16进制数的起始地址
//	[IN] nLen - 16进制数的字节数
// return value:
// remarks : 将16进制数转化为字符串
*/
void HexToStr(unsigned char *pbDest, unsigned char *pbSrc, int nLen);
void MD5Print(unsigned char * digest);
void PrintHex(unsigned char * data, int len);
void PrintMACAddress(unsigned char MACData[]);

/*
    char data[100];
    unsigned char * pbSrc1 = (unsigned char *)"21690d95033620379475a9d8f8dac222";
    unsigned char pbDest1[16];
    data[0] = (char)192;
    u_char* digest;
    memcpy(data + 1, "14319Xzte142052", 6 + 9);
    StrToHex(pbDest1, pbSrc1, 16);
    memcpy(data + 16, pbDest1, 16);
    digest = Get_Md5_Digest((u_char*)data, 32);
    MD5Print(digest);
    //29ed270fd8edcb087424237870d578f4
    */

    //unsigned char digest[16];
    /*unsigned char key[256] = {"123"};
    unsigned char pData[512] = "admin";

    cout << endl;
    rc4_crypt(pData, 5, key, 3);
    printf("%s\n", pData);*/
    //unsigned char * data = (unsigned char *)"u14319x";
    //md5(data, sizeof(data), digest);
    //hmac_md5((unsigned char*)"admin", 5, (unsigned char*)"123", 3, digest);
    //MD5Print(digest);
    //cout << endl;

    /*unsigned char enckey[] = { 0x02, 0x02, 0x14, 0x00 };
    unsigned char wholekey[20];
    unsigned char * pbSrc2 = (unsigned char *)"efa90d7da0f4b7cca1bb4067c82c9969c82c9969";
    unsigned char pbDest2[20];
    StrToHex(pbDest2, pbSrc2, 20);
    PrintHex(pbDest2, 20);
    memcpy(wholekey, pbDest2, 20);
    Get_RC4(enckey, 4, wholekey, 20);
    PrintHex(enckey, 4);*/
    /*unsigned char * pbSrc = (unsigned char *)"323031353335303230343231"; //201535020421
    unsigned char pbDest[16];
    memset(pbDest, 0, 16);
    StrToHex(pbDest, pbSrc, 12);
    printf("%s", pbDest);*/

    /*unsigned char encDat[64];
    memset(encDat, 0, 64);
	unsigned char * deckey;
	unsigned char * pbSrc3 = (unsigned char *)"";
	unsigned char pbDest3[52];
	StrToHex(pbDest2, pbSrc2, 52);
	memcpy(encDat, pbDest3, 52);
	enckey[0] = encDat[31];
	deckey = Get_Hmac_Md5_Digest(encDat, 52, enckey, 1);
	memcpy(eap_life_keeping + 46, deckey, 16);
	PrintHex(deckey, 16); */


#endif // ADAPTER_H_
