#include "adapter.h"
#include <unistd.h>

bool GetMacAddress(const char* adapterName, unsigned char* mac)
{
#ifndef WIN32
    struct ifreq ifreq;
    int sock;
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return false;
    strcpy(ifreq.ifr_name, adapterName);
    if(ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
        return false;
    memcpy((void *)mac, (void *)(ifreq.ifr_hwaddr.sa_data), 6);
    return true;
#else
    PIP_ADAPTER_INFO AdapterInfo = NULL;
    DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
    DWORD dwStatus;

    AdapterInfo = (PIP_ADAPTER_INFO)malloc(dwBufLen);
    if (AdapterInfo == NULL) return false;

    dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);

    if (dwStatus == ERROR_BUFFER_OVERFLOW)
    {
        free(AdapterInfo);
        AdapterInfo = (PIP_ADAPTER_INFO)malloc(dwBufLen);
        dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
        if (AdapterInfo == NULL) return -1;
    }

    if (dwStatus != NO_ERROR)
    {
        if (AdapterInfo != NULL) free(AdapterInfo);
        return false;
    }

    //if (ip) memset(ip, 0, 16);
    if (mac) memset((char*)mac, 0, 6);

    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;

    if (adapterName != NULL)
    {
        while (pAdapterInfo)
        {
            //printf("%s\n", pAdapterInfo->AdapterName);
            //PrintMACaddress(pAdapterInfo->Address);
            if (strcmp(adapterName, pAdapterInfo->AdapterName) == 0 || !adapterName)
            {
                if (mac) memcpy((char*)mac, pAdapterInfo->Address, 6);
                //if (ip) strncpy(ip, pAdapterInfo->IpAddressList.IpAddress.String, 16);
                break;
            }
            pAdapterInfo = pAdapterInfo->Next;
        }
    }

    free(AdapterInfo);
    if (pAdapterInfo == NULL) return -3;
    return true;
#endif
}

bool GetAdapterName(pcap_if_t *device, char* adapterName)
{
#ifndef WIN32
    char* name = device->name;
#else
    char* name = device->name + 12;
#endif
    strncpy(adapterName, name, 100);
    return true;
}


bool GetIp(const char * adapterName, unsigned char * ip)
{
    pcap_if_t *alldevs;
    pcap_if_t *d;
    pcap_addr_t *a;
    char name[100];
    u_char *p  = NULL;
    char errbuf[PCAP_ERRBUF_SIZE];

    AdapterNameToPcap(adapterName, name);
    // 获取本机设备列表
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
        return false;
    for(d=alldevs; d; d=d->next)
        if(!strcmp(d->name, name)) break;
    if(!d) return false;
    for(a = d->addresses; a; a = a->next)
    {
        if(a->addr->sa_family == AF_INET)
        {
            if (a->addr)
            {
                p = (u_char *)&(((struct sockaddr_in *)a->addr)->sin_addr.s_addr);
                break;
            }
        }
    }
    if(p)
    {
        sprintf((char *)ip, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
        return true;
    }
    else
        return false;
}

bool AdapterNameToPcap(const char * adapterName, char * pcapAdapterName)
{
    pcapAdapterName[0] = '\0';
#ifdef WIN32
    const char *pre = "\\Device\\NPF_";
    strncpy(pcapAdapterName, pre, 14);
#endif // WIN32
    strncat(pcapAdapterName, adapterName, 100);
    return true;
}

int GetAdapter(char ** adapterName)
{
    static char name[100];
    pcap_if_t *alldevs;
    pcap_if_t *d;
    char errbuf[PCAP_ERRBUF_SIZE];
    int i = 0, inum, t;

    // 获取本机设备列表
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        fprintf(stderr,"Error: pcap_findalldevs - %s\n", errbuf);
        exit(1);
    }

    // 打印列表
    for(d=alldevs; d; d=d->next)
    {
        printf("%d. %s", ++i, d->name);
        if (d->description)
            printf(" (%s)\n", d->description);
        else
            printf(" (No description available)\n");
    }

    if(i==0)
    {
        printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
        return -1;
    }

    printf("Enter the interface number (1-%d):",i);
    t = scanf("%d", &inum);
    if(t) {  }

    if(inum < 1 || inum > i)
    {
        printf("\nInterface number out of range.\n");
        pcap_freealldevs(alldevs);
        return -1;
    }

    // 跳转到选中的适配器
    for(d=alldevs, i=0; i< inum-1 ;d=d->next, i++);
    GetAdapterName(d, name);
    *adapterName = name;
    pcap_freealldevs(alldevs);
    return 0;
}


bool DhcpClient(const char *adapterName, int style, const char *dhcpScript)
{
    pid_t fpid;
    char * const dhclientArgv[] = {(char *)"dhclient", (char *)adapterName, 0};
    char * const udhcpcArgv[] = {(char *)"udhcpc", (char *)"-p /tmp/dclient_dhcp.pid",(char *)"-n", (char *)"-i",
                                 (char *)adapterName, (char *)"-t1", (char *)"-R", 0}; // dhcpScript暂时用不到
    fpid = fork(); // 执行udhcpc会使进程退出，因此fork来执行
    if(fpid > 0) // 父进程返回
        return true;
    else if(fpid == 0)
    {
        // 子进程dhcp
        if(style == 1)
            execvp("dhclient", dhclientArgv);
        else if(style == 2)
            execvp("udhcpc", udhcpcArgv);
    }
    return true;
}


/*
// C prototype : void StrToHex(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 输出缓冲区
//	[IN] pbSrc - 字符串
//	[IN] nLen - 16进制数的字节数(字符串的长度/2)
// return value:
// remarks : 将字符串转化为16进制数
*/
void StrToHex(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
{
    char h1,h2;
    unsigned char s1,s2;
    int i;

    for (i=0; i<nLen; i++)
    {
        h1 = pbSrc[2*i];
        h2 = pbSrc[2*i+1];

        s1 = toupper(h1) - 0x30;
        if (s1 > 9)
            s1 -= 7;

        s2 = toupper(h2) - 0x30;
        if (s2 > 9)
            s2 -= 7;

        pbDest[i] = s1*16 + s2;
    }
}

/*
// C prototype : void HexToStr(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 存放目标字符串
//	[IN] pbSrc - 输入16进制数的起始地址
//	[IN] nLen - 16进制数的字节数
// return value:
// remarks : 将16进制数转化为字符串
*/
void HexToStr(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
{
    char ddl,ddh;
    int i;

    for (i=0; i<nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57) ddh = ddh + 7;
        if (ddl > 57) ddl = ddl + 7;
        pbDest[i*2] = ddh;
        pbDest[i*2+1] = ddl;
    }

    pbDest[nLen*2] = '\0';
}


void MD5Print(unsigned char * digest)
{
    for (int i = 0; i < 16; i++)
        printf("%02X", digest[i]);
    printf("\n");
}

void PrintHex(unsigned char * data, int len)
{
	int i = 0;
	printf("\nlen : %d\n", len);
    for (i = 0; i < len; i++)
    {
        printf("%02X ", data[i]);
        if(i % 8 == 7) printf(" ");
        if(i % 16 == 15) printf("\n");
    }
    printf("\n");
}

void PrintMACAddress(unsigned char MACData[])
{
    printf("MAC Address: %02X-%02X-%02X-%02X-%02X-%02X\n",
    MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
}
