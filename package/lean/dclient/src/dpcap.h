#ifndef DPCAP_H_
#define DPCAP_H_

#include <ctime>
#include <string>
#include <pthread.h>
//#include <signal.h>
#include "global.h"
#include "msgpass.h"
#include "eappacket.h"

class DPcap : public EapPacket
{
public:
    enum EapType                            // 消息类型
    {
        EAPOL_START = 1,                    // EAPOL START
		EAPOL_LOGOFF,                       // EAPOL LOGOFF
		EAP_REQUEST_IDENTITY,               // EAP Request, Identity
		EAP_RESPONSE_IDENTITY,              // EAP Response, Identity
		EAP_REQUEST_IDENTITY_KEEP_ALIVE,    //
		EAP_RESPONSE_IDENTITY_KEEP_ALIVE,   //
		EAP_REQUEST_MD5_CHALLENGE,          // EAP Request, MD5-Challenge
		EAP_RESPONSE_MD5_CHALLENGE,         // EAP Response, MD5-Challenge
		EAP_SUCCESS,                        // EAP Success
		EAP_FAILURE,                        // EAP Failure
		EAP_KEEP_ALIVE,                     // EAPOL Key
		ZTE_EAPOL_MSG,                      // EAP Request, Notification
		EAP_ERROR,                          //
		ZTE_MORE_MSG
    };

    enum ErrorType
    {
        UNKOWN = 0,
        ZTE_021 = 021, // 多网卡错误
        ZTE_100 = 100, // 未知错误
        ZTE_101 = 101, // 不存在的用户
        ZTE_102 = 102, // 密码错误
        ZTE_109 = 109, // 非私有认证 10s
        ZTE_110 = 110, // 用户在线限制 60s
        ZTE_207 = 207, // 没有接入权限
    };

    enum EapolType
    {
        EAP_PACKET = 0,
        //EAPOL_START = 1,
        //EAPOL_LOGOFF = 2,
        EAP_KEY = 3
    };

    DPcap(MsgPass * msgPass = (MsgPass *)NULL);
    ~DPcap();
    void pcapProc(const struct pcap_pkthdr* packet_header, const u_char* pkt_data);
    void init(const std::string & user, const std::string & passwd,
              const std::string & adapterName);
    bool open();
    void run();
    bool close();
    int sendPacket(EapType send_type);
    int getStatus();
    void setStatus(EapType type);
    void setStatusAndRun(EapType type);
    void setMessage(const std::string & msg);
    std::string getMessage();
    int getEapType();
    std::string getAdapterName();
    bool getRunStatus();
    long getPreRVal();    // 获取上次到调用时，与服务器上次交互认证信息的间隔时间
    time_t getReSendTime(); // 获取下一次发送start包的时间，获取之后如果没有新的再次获取会得到一个负数
    void setReSendTime(int s); //参数是秒数，重新发包的时间为当前时间+秒数

protected:
    bool ready;         // 报文信息初始化状态
    bool isOpen;        // pcap状态
    EapType status;     // Pcap执行情况
    bool canRun;        // 收到包时是否回复
    MsgPass * msgPass;  // 日志记录
    time_t  preRun;     // 服务器最后发包时间
    time_t  nextTime;   // 下一次重新发包时间
    std::string message; // 暂时存放的消息
    pthread_mutex_t mutex_status;
    pthread_mutex_t mutex_msg;

    bool suspend(int s);
    void setSuspend();
    int getSuspendTime(bool increase = false);
    void processPacket();
private:
    pcap_t *adhandle;
    std::string adapterName;
};

#endif // DPCAP_H_
