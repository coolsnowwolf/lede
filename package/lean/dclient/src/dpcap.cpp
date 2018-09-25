#include <ctime>
#include <cstring>
#include "global.h"
#include "dpcap.h"
#include "adapter.h"

#include <iostream>
using std::cout;
using std::endl;
using std::string;

//static void ReSendStartPacket(int sig) // 定时器调用
//{
//    return ;
//}

static void packet_handler(u_char *lp, const struct pcap_pkthdr* header, const u_char* pkt_data)
{
    DPcap * dpcap = (DPcap *)lp;
    if(dpcap) dpcap->pcapProc(header, pkt_data);
    return;
}

DPcap::DPcap(MsgPass * msgPass)
{
    ready = false;
    isOpen = false;
    canRun = false;
    nextTime = -1;
    message = "";
    if(msgPass) this->msgPass = msgPass;
    pthread_mutex_init(&mutex_status, NULL);
    pthread_mutex_init(&mutex_msg, NULL);
}

DPcap::~DPcap()
{
    pthread_mutex_destroy(&mutex_msg);
    pthread_mutex_destroy(&mutex_status);
}

void DPcap::init(const std::string & user, const std::string & passwd, const std::string & adapterName)
{
    uint8_t mac[6];
    this->adapterName = adapterName;
    GetMacAddress(adapterName.c_str(), mac);    // 获取Mac地址
    EapPacket::init(user, passwd, mac);         // 交给基类初始化
    ready = true;
    //signal(SIGALRM, ReSendStartPacket);       // //注册安装定时信号

    // msg在基类声明定义, char msg[300]
	sprintf(msg, "[Init] Interface: %s\n"
           "\t\t  [Init] MAC Address: %02X-%02X-%02X-%02X-%02X-%02X\n"
           "\t\t  [Init] 认证模块初始化完成...",
    adapterName.c_str(), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    setMessage(msg);
    return;
}

// 打开Pcap设备
bool DPcap::open()
{
    char name[100];
    char FilterStr[100];
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fcode;

    if(!ready) return false; // 防止未初始化调用
    if(isOpen) return true;  // 防止多次调用

    AdapterNameToPcap(adapterName.c_str(), name);
    if(!(adhandle = pcap_open_live(name, 65536, 1, 2000, errbuf)))
    {
        setStatus(EAP_ERROR);
        sprintf(msg,"[Pcap Error] Unable to open the adapter. %s is not supported.", name);
        setMessage(msg);
        return false;
    }

    // -------------日志记录-------------- //
    sprintf(msg,"[Pcap] 正在打开设备...");
    setMessage(msg);
    // ---------------End---------------- //

    // 确保设备正确
	if(pcap_datalink(adhandle) != DLT_EN10MB)
    {
        setStatus(EAP_ERROR);
		sprintf(msg, "[Pcap Error] %s is not an Ethernet.", name);
		setMessage(msg);
		return false;
	}

	sprintf(FilterStr, "(ether proto 0x888e) and (ether dst host %02x:%02x:%02x:%02x:%02x:%02x)",
		local_mac[0], local_mac[1], local_mac[2], local_mac[3], local_mac[4], local_mac[5]);

    // -------------日志记录-------------- //
    //sprintf(msg,"[Pcap] 正在设置过滤器...");
    //setMessage(msg);
    //if(msgPass) msgPass->signal();
    // ---------------End---------------- //

    // 编译过滤器
    if(pcap_compile(adhandle, &fcode, FilterStr, 1, 0xff) < 0 )
    {
        setStatus(EAP_ERROR);
        sprintf(msg,"[Pcap Error] Unable to compile the packet filter.");
        setMessage(msg);
        return false;
    }

    // 设置过滤器
    if(pcap_setfilter(adhandle, &fcode) < 0)
    {
        setStatus(EAP_ERROR);
        sprintf(msg,"[Pcap Error] Error setting the filter.");
        setMessage(msg);
        return false;
    }

    canRun = true;
    return isOpen = true;
}

//运行
void DPcap::run()
{
    sendPacket(EapType::EAPOL_START);
    pcap_loop(adhandle, 0, packet_handler, (u_char *)this);
}

bool DPcap::close()
{
    pcap_close(adhandle);
    isOpen = false;
    canRun = false;
    return true;
}

void DPcap::pcapProc(const struct pcap_pkthdr* packetinfo, const u_char* packet)
{
    preRun = time(0);
    this->packet = (uint8_t *)packet;
    this->eap_head = (struct eap_header*)(packet + SIZE_ETHERNET);
    processPacket();    // 处理报文数据
}

void DPcap::processPacket()
{
    EapType type;
    if(!canRun) return;         // 判断是否可以处理服务器发来的报文
    type = (EapType)getEapType();
    if(type == EAP_ERROR) return;

    std::string tmp = "";
    string::size_type pos;
    switch(type)
    {
    case EAPOL_START:
        // 此类型的报文是发出去的，不会由服务器发来
        // 此处在必要的时候用来调试发包是否成功
        break;
    case EAPOL_LOGOFF:
        notificationMessage();
        tmp = "[Logoff] ";
        tmp.append(msg);
        setMessage(tmp);
        setStatus(EAPOL_LOGOFF);
        //  把消息记录进日志
        break;
    case EAP_SUCCESS:
        //suspend(1); // 成功的报文不需要回复，腾出时间给其他线程记录日志
        setMessage("[Success] 中兴802.1X认证登录成功！");
        setStatus(EAP_SUCCESS);
        break;
    case EAP_FAILURE:
        severInfo();
        tmp = msg; // 临时保存下msg的内容
        if((pos = tmp.find(":")) != string::npos)
            tmp = tmp.substr(pos + 1); // 截取冒号后面的字符串
        else
            tmp = "";
        if(tmp.length() > 0) {
            sprintf(msg, "[Failure] 认证失败，%s。将在%d秒后重试。", tmp.c_str(), getSuspendTime(false));
        } else {
            sprintf(msg, "[Failure] 认证失败，原因未知。将在%d秒后重试。", getSuspendTime(false));
        }
        tmp = msg;
        setMessage(tmp);
        setStatus(EAP_FAILURE);
        //suspend(1);     // 暂停1s，让出cpu时间记录下日志，注意suspend后会设置canRun为true
        canRun = false;
        setSuspend();   // 收到认证失败的包之后，一段时间再重试
        //eapFailure();
        break;
    case EAP_REQUEST_IDENTITY:
        responseIdentity();
        sendPacket(EAP_RESPONSE_IDENTITY);
        break;
    case EAP_REQUEST_MD5_CHALLENGE:
        responseMd5Challenge();
        sendPacket(EAP_RESPONSE_MD5_CHALLENGE);
        break;
    case ZTE_EAPOL_MSG:
        notificationMessage();
        tmp = "[Notification] ";
        tmp.append(msg);
        //setMessage(tmp);    // 由于failure通知也有相关信息，必要时可屏蔽notification通知
        setStatus(ZTE_EAPOL_MSG);
        break;
    case EAP_KEEP_ALIVE:
        keepAlive();
        sendPacket(EAP_RESPONSE_IDENTITY_KEEP_ALIVE);
        break;
    default:
        break;
    }
}

// 发送数据包
int DPcap::sendPacket(enum EapType send_type)
{
#ifdef DEBUG
    static int cnt = 0;
#endif
    static uint8_t* frame_data;
    int              frame_len = 0;
    if(!canRun) return 0;
    preRun = time(0);
    std::string tmp;
    switch(send_type)
    {
    case EAPOL_START:
        frame_data = eapol_start;
        frame_len = sizeof(eapol_start);
        setMessage("[Start] 开始中兴802.1X认证...");
        setStatus(EAPOL_START);
        break;
    case EAPOL_LOGOFF:
        frame_data = eapol_logoff;
        frame_len = sizeof(eapol_logoff);
        setMessage("[Cancel] 正在取消中兴认证登录...");
        setStatus(EAPOL_LOGOFF);
        break;
    case EAP_RESPONSE_IDENTITY:
        frame_data = eap_response_ident;
        frame_len = sizeof(eap_response_ident);
#ifdef DEBUG
        setMessage("[Identity] 向服务器发送用户名...");
#endif
        break;
    case EAP_RESPONSE_MD5_CHALLENGE:
        frame_data = eap_response_md5ch;
        frame_len = sizeof(eap_response_md5ch);
#ifdef DEBUG
        setMessage("[Md5Challenge] 正在验证身份...");
#endif
        break;
    case EAP_RESPONSE_IDENTITY_KEEP_ALIVE:
        frame_data = eap_life_keeping;
        frame_len = sizeof(eap_life_keeping);
#ifdef DEBUG
        if(cnt++ % 20 == 1) setMessage("[Keep Alive] 中兴心跳包...");
#endif
        break;
    default:
        return 3;
        break;
    }

    if(pcap_sendpacket(adhandle, frame_data, frame_len) != 0)
    {
        //sprintf(msg, "Error Sending the packet: %s\n", pcap_geterr(adhandle));
        tmp = "[Error] Sending the packet: ";
        tmp.append(pcap_geterr(adhandle));
        setMessage(tmp);
        setStatus(EAP_ERROR);
        setSuspend();
        return 0;
    }
    return 1;
}

int DPcap::getEapType()
{
    if (eap_head->eapol_t == EAP_KEY)
		return EAP_KEEP_ALIVE;

	switch (eap_head->eap_t)
	{
	case 0x00:
		if (eap_head->eap_op == 0x1c &&
			eap_head->eap_id == 0x00)
			return ZTE_EAPOL_MSG;
		break;
	case 0x01:
		if (eap_head->eap_op == 0x01)
			return EAP_REQUEST_IDENTITY;
		if (eap_head->eap_op == 0x02)
			return ZTE_EAPOL_MSG;
		if (eap_head->eap_op == 0x04)
			return EAP_REQUEST_MD5_CHALLENGE;
		break;
	case 0x03:
		return EAP_SUCCESS;
		break;
	case 0x04:
		return EAP_FAILURE;
		break;
    default:
        return -1;
	}
	return -1;
}

std::string DPcap::getAdapterName()
{
    return adapterName;
}

int DPcap::getStatus()
{
    int s = 0;
    pthread_mutex_lock(&mutex_status);
    s = status;
    pthread_mutex_unlock(&mutex_status);
    return s;
}

void DPcap::setStatus(EapType type)
{
    pthread_mutex_lock(&mutex_status);
    status = type;
    pthread_mutex_unlock(&mutex_status);
    return;
}

void DPcap::setStatusAndRun(EapType type)
{
    switch(type)
    {
    case EAPOL_START:
        canRun = true;
        sendPacket(EapType::EAPOL_START);
        break;
    case EAPOL_LOGOFF:
        sendPacket(EapType::EAPOL_LOGOFF);
        canRun = false;
        break;
    case EAP_FAILURE:
        canRun = false;
        break;
    default:
        break;
    }
}

void DPcap::setMessage(const std::string & msg)
{
    pthread_mutex_lock(&mutex_msg);
    message = msg;
    if(msgPass) msgPass->signal();
    pthread_mutex_unlock(&mutex_msg);
    return;
}

std::string DPcap::getMessage()
{
    pthread_mutex_lock(&mutex_msg);
    std::string str = message;
    pthread_mutex_unlock(&mutex_msg);
    return str;
}

bool DPcap::suspend(int s)
{
    canRun = false;
    if(s <= 0) return true;
#ifndef WIN32
    ::sleep(s);
#else
    ::Sleep(s * 1000);
#endif
    canRun = true;
    return true;
}

void DPcap::setSuspend()
{
    int s = getSuspendTime(true);
    //char info[100];
    //sprintf(info, "[Message] 认证失败,将在%d秒后重试...", s);
    //setMessage(info);
    //if(msgPass) msgPass->signal();
    if(s == 0) // 0秒立即
    {
        canRun = true;
        setStatusAndRun(EapType::EAPOL_START);
    }else
    {
        canRun = false;
        //alarm(s);      //触发定时器
        setReSendTime(s);
    }
    return ;
}

// 认证出错时，重试的间隔时间算法
int DPcap::getSuspendTime(bool increase) // 如果increase是false(默认)，重试的时间不会增长
{
    static time_t preTime = 0;
    static int k = 0;

    time_t now = time(0);
    const int rank[9] = {0, 5, 15, 30, 65, 300, 600, 900, 1200}; // 各个等级需要暂停的时间
    int rVal = 0;
    long interval = now - preTime;

    if(interval > 1800 || k < 0) k = 0;
    rVal = rank[k];
    if(increase)
    {
        if(k < 8) k++;
        preTime = now;
    }
    return rVal;
}

bool DPcap::getRunStatus()
{
    return canRun;
}

long DPcap::getPreRVal()
{
    time_t now = time(0);
    return (now - preRun);
}

void DPcap::setReSendTime(int s)
{
    this->nextTime = time(0) + s;
}

time_t DPcap::getReSendTime()
{
    time_t t;
    t = this->nextTime;
    this->nextTime = -1;
    return t;
}
