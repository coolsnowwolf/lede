#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <string>
//#include <regex> // C++
#include "global.h"
#include "dpcap.h"
#include "tprocess.h"
#include "adapter.h"
#include "docr.h"
#include "encrypt.h"

using std::string;
using std::cout;
using std::endl;

using namespace std;


static void * RunDPcap(void * lpVoid)
{
    TProcess * tp = (TProcess *)(lpVoid);
    tp->runDPcap();
    return NULL;
}

static void * RunDPcapWait(void * lpVoid)
{
    TProcess * tp = (TProcess *)(lpVoid);
    tp->runDPcapWait(); //string::size_type pos;
    return NULL;
}

static void * DPcapWaitProc(void * lpVoid)
{
    TProcess * tp = (TProcess *)(lpVoid);
    tp->log(tp->getDPcapHandle()->getMessage());
    return NULL;
}


static void * RunDCurl(void * lpVoid)
{
    TProcess * tp = (TProcess *)(lpVoid);
    tp->runDCurl();
    return NULL;
}

static void * RunDetectActive(void *lpVoid)
{
    TProcess * tp = (TProcess *)(lpVoid);
    tp->runDetectActive();
    return NULL;
}

TProcess::TProcess(const string & zteUser, const string & ztePass, const string & adapterName,
                   int dhcpStyle, string dhcpScript, int timeout, int enetAuth,
                   const string & enetUser, const string & enetPass)
{
    this->zteUser = zteUser; this->ztePass = ztePass;
    this->adapterName =adapterName; this->dhcpStyle = dhcpStyle;
    this->dhcpScript = dhcpScript; this->timeout = timeout;
    this->enetAuth = enetAuth; this->enetPass = enetPass; this->enetUser = enetUser;
    this->change = 0;
    //pthread_mutex_init(&mutex_log, NULL); //日志函数用的，已注释
    // TODO(Zeyes) 在上线前，把文件改到临时目录
    //logFile = "/tmp/zte.log"; // 日志文件名
    dlog = new DLog("/tmp/zte.log"); // 初始化日志记录
    msgPass = new MsgPass();         // 初始化中兴认证消息传递模块
}

TProcess::~TProcess()
{
    //pthread_mutex_destroy(&mutex_log); //日志函数用的，已注释
}

// 多线程总调度
int TProcess::run()
{
    pthread_t eThread[4];
    pthread_attr_t attr[4];
    int rc;
    void *status;

    pthread_attr_init(&attr[0]);
    pthread_attr_setdetachstate(&attr[0], PTHREAD_CREATE_JOINABLE);

    rc = pthread_create(&eThread[0], &attr[0], RunDPcap, (void *)this);  // 中兴认证线程
    pthread_create(&eThread[1], NULL, RunDPcapWait, (void *)this);       // 中兴认证消息记录线程
    pthread_create(&eThread[2], &attr[0], RunDCurl, (void *)this);       // 天翼校园网和外网心跳进程
    pthread_create(&eThread[3], &attr[0], RunDetectActive, (void *)this);// 检测中兴认证状态与重置
    pthread_join(eThread[0], &status);

    log("[Exit] 认证程序异常退出!");

    return rc;
}

// 运行中兴认证
void TProcess::runDPcap()
{
    if(msgPass) dpcap = new DPcap(msgPass); //msgPass是一个日志记录实例指针
    else dpcap = new DPcap();
    dpcap->init(zteUser, ztePass, adapterName);
    dpcap->open();
    dpcap->run();
    //test->setStatus(DPcap::EAPOL_LOGOFF);
}

// 接收Pcap发来的消息，并调用本类日志记录函数
void TProcess::runDPcapWait()
{
    //for(int i = 0; i < 1000; i++) if(dpcap) break;
    while(1)
    {
        if(msgPass)
        {
            msgPass->wait(DPcapWaitProc, (void *)this);
            change = 1;         // 表明收到消息，可能有状态的改变
        }
    }
}

void TProcess::runDCurl()
{
    unsigned char ip[17];            // 保存ip地址
    int nowStatus = -10, preStatus = -10; // 保存中兴认证的状态和前一次的状态
    int maxLoginCount = 3;       // 天翼校园网失败最大重试次数
    bool loginStatus = false;     // 天翼校园网登录状态
    static int loginFailCount = 0;  // 天翼校园网失败尝试计数
    int cnt = 0;                 // 统计秒数，非精确值
    int hcnt = 0;                // 心跳错误统计次数
    int rcnt = 0;                // 局域网心跳计数
    int ipcnt = 0;               // ip获取失败计数
    char temp[10];

    suspend(5);
    while(1)
    {
        suspend(1); cnt++; cnt %= 60; // 秒数计数

        if(!dpcap) continue;
        if(!dcurl)
        {
            dcurl = new DCurl();
            dcurl->setTimeout(20, 5);
            continue;
        }

        preStatus = nowStatus;
        nowStatus = dpcap->getStatus(); // 获取中兴认证状态
        if(nowStatus != preStatus || change) // 中兴状态发生改变或者有change信息产生
        {
            change = 0;                  // 成功调用后，将change改回0
            if(!loginStatus && nowStatus == DPcap::EAP_SUCCESS) // 中兴认证成功，并且外网没有登录
            {
                const char * name = (dpcap->getAdapterName()).c_str();
                if(!GetIp(name, ip)) // 第一次检测ip是否获取成功
                {
                    if(ipcnt % 600 == 0) log("[Proc] 等待获取ip...");
                    if(dhcpStyle && cnt % 3 == 0)
                        DhcpClient(name, dhcpStyle, dhcpScript.c_str()); // 如果没有获取ip，就按照参数获取ip，默认是dhcpStyle=0
                }

                if(!GetIp(name, ip)) // 第二次检测ip是否获取成功
                    { if(ipcnt % 600 == 120) log("[Error] ip未成功获取!"); ipcnt++; change = 1; continue; }
                else
                    { if(loginFailCount == 0) log("[Info] 当前ip - " + string((const char *)ip)); if(ipcnt) ipcnt = 0; }

                if(!loginStatus && runHeartbean(true)) // 检测需不需要登录外网
                {
                    log("[Info] 检测到已经连通互联网...");
                    loginStatus = true;
                    loginFailCount = 0;
                    maxLoginCount = 3;
                    if(rcnt) rcnt = 0; if(hcnt) hcnt = 0;
                }

                if(enetAuth && !loginStatus) // 登录状态不是为真时执行
                {
                    // 不是第一次尝试登录时打印log
                    if(0 < loginFailCount && loginFailCount < maxLoginCount)
                    {
                        sprintf(temp, "%d", loginFailCount + 1);
                        log("[Enet Retry] 进行第" + string(temp) + "次尝试...");
                    }
//#endif // DEBUG
                    // 如果天翼网尝试次数小于最大可登录次数
                    if(loginFailCount < maxLoginCount)
                    {
                        // 执行登录天翼外网
                        if(runLoginEnet(ip))
                        {
                            loginStatus = true;
                            loginFailCount = 0;    // 将天翼校园网重试次数重置
                            maxLoginCount = 3;
                            if(rcnt) rcnt = 0; if(hcnt) hcnt = 0;
                        }else
                        {
                            change = 1;
                            loginFailCount++;
                        }
                    }
                }

            }
            else if(nowStatus != DPcap::EAP_SUCCESS && nowStatus != DPcap::EAP_KEEP_ALIVE)
            {
                // 中兴状态发生改变而且不是成功的状态时
                if(enetAuth && loginStatus) log("[Enet] 已重置天翼校园网登录状态...");
                if(loginStatus) loginStatus = false;
                if(loginFailCount) loginFailCount = 0;    // 将天翼校园网重试次数重置
                maxLoginCount = 3;
                if(hcnt) hcnt = 0; if(rcnt) rcnt = 0;
            }
        }

        // 只要dpcap实例存在，如果天翼校园网认证成功或者已经获取到ip都进行心跳
        // 实际上就算天翼校园网认证不成功也要心跳，不然中兴认证也会断(需要有数据包交换)
        if(cnt == 59)
        {
            if(loginStatus)
            {
                if(!runHeartbean(true)) // 心跳60秒一次，验证返回信息
                {
                    hcnt += 1;
                    if(hcnt == 4) log("[Error] 天翼网心跳连接失败！");
                }else
                    hcnt = 0;
            }else if(nowStatus == DPcap::EAP_SUCCESS && loginFailCount >= maxLoginCount && GetIp(adapterName.c_str(), ip)) // 登录失败时改为内网心跳
            {
                rcnt += 1;                    // 局域网心跳计数+1
                if(runHeartbean(true))
                    change = 1;            // 用户手动登录天翼校园网的情况
                else
                    runHeartbean(false, true); // 局域网心跳
                if((rcnt % 15 == 14) && enetAuth) { maxLoginCount+=1; change = 1; }
                // 如果一直没有认证成功，每15分钟给一次重新认证的机会
            }

            // 天翼校园网无故断线重连
            if(hcnt % 5 == 4)
            {
                if(loginFailCount == 0) log("[Retry] 天翼网可能已断开，正在重新连接...");
                loginStatus = false; change = 1;
                if(loginFailCount >= maxLoginCount) hcnt = 0;
                else hcnt %= 5;
            }
        }
    }

    delete dcurl;
    dcurl = NULL;
    return ;
}

// toVar表示是否验证返回内容，local表示是否局域网心跳, 默认为false
bool TProcess::runHeartbean(bool toVar, bool local)
{
    std::string url = "http://www.msftncsi.com/ncsi.txt"; // 心跳网址
    std::string pattern = "Microsoft NCSI";            // 心跳网址的返回内容，用于判断心跳是否成功
    if(local) { url = "http://10.20.208.12/default3.aspx"; pattern = "html"; }
    if(!toVar)
    {
        if(dcurl)
            dcurl->httpRequest(url, DCurl::GET, false);
    }
    else
    {
        std::string strResponse = "";
        if(dcurl)
        {
            dcurl->httpRequest(url, DCurl::GET, true);
            dcurl->getResponse(strResponse);
            if(strResponse.find(pattern) != strResponse.npos) return true;
            else return false;
            /*std::regex idRegex("html");
            auto itBegin = std::sregex_iterator(strResponse.begin(), strResponse.end(), idRegex);
            auto itEnd = std::sregex_iterator();
            std::smatch match = *itBegin;
            if(match.length() >= 4 && strstr(match[0].str().c_str(), "html")) return true;
            else return false;*/
        }
        else
            return false;
    }

    return true;
}

bool TProcess::runLoginEnet(const unsigned char * ip)
{
    string strResponse = "";
    string code = "";
    string postStr;
    string tmp;
    DOcr * valite = new DOcr();
    bool result = false;
    bool resOcr = false;

    unsigned char * password1 = (unsigned char *)enetPass.c_str();
    if(dcurl)
    {
        string t_str;
        dcurl->httpRequest("http://enet.10000.gd.cn:10001/login.jsp", DCurl::POST, true);
        dcurl->getResponse(t_str);
        if(t_str.find("base64") != string::npos)    // 检测天翼登录页是否引入了base64.js
        {
            password1 = (unsigned char *) calloc((enetPass.length() + 2) / 3 * 4, sizeof(unsigned char));
            Get_Base64_Encode((unsigned char *) enetPass.c_str(), enetPass.length(), password1); // 天翼密码Base64加密
        }
    }

    for(int i = 0; !result && i < 2; i++)
    {
        strResponse = "";
        code = "";
        if(!dcurl) return false;
        dcurl->httpRequest("http://enet.10000.gd.cn:10001/common/image.jsp", DCurl::GET, true);
        dcurl->getResponse(strResponse);
        if(!strResponse.empty() && strResponse.size() >= 800 && valite)
        {
            resOcr = valite->readImage(strResponse, DOcr::JPEG_MEM);
            if(resOcr) valite->run();
            if(resOcr) valite->getCode(code);
        }

        postStr = "userName1=" + enetUser + "&password1=" + string((char *)password1) + \
                "&rand=" + code + "&eduuser=" + string((const char *)ip) + "&edubas=113.98.10.136";
        //cout << "postStr: " << postStr << endl;
        dcurl->setPostData(postStr);
        dcurl->httpRequest("http://enet.10000.gd.cn:10001/login.do", DCurl::POST, true);
        dcurl->getResponse(strResponse);
        //log(strResponse);
        // gcc 4.8的版本不支持C++ 11的正则表达式
        /*
        std::regex idRegex("<div id=\"(.*?)\">(.*?)<");
        auto itBegin = std::sregex_iterator(strResponse.begin(), strResponse.end(), idRegex);
        auto itEnd = std::sregex_iterator();
        std::smatch match = *itBegin;

        if(match.length() > 10 && strstr(match[0].str().c_str(), "success"))
        {
            tmp = "[Enet] ";
            tmp.append(match[0].str() + string(" - ") + match[2].str());
            log(tmp);
            result = true;
        }
        else
        {
            tmp = "[Enet Faild] ";
            if(match.length() > 10) tmp.append(match[0].str() + string(" - ") +  match[2].str());
            else tmp.append("天翼校园网登陆失败！");
            log(tmp);
            if(runHeartbean(true)) result = true;
        }
        */

        // 下面为妥协版
        string::size_type pos;
        if((pos = strResponse.find("success")) != string::npos)
            result = true;
        else
            if(runHeartbean(true)) result = true;

        if(!result) suspend(2);
        //if(runHeartbean(true)) result = true;
    }
    if(valite) { delete valite; valite = NULL; } // 删除验证码识别对象内存

    // 日志记录登录情况
    if(result == true)
        log("[Enet] 天翼校园网登录成功!");
    else
    {
        string::size_type pos1 = string::npos, pos2 = string::npos;
        if(!strResponse.empty())
        {
            pos1 = strResponse.find("failed") + 8;
            pos2 = strResponse.find("</div>");
        }
        if(pos1 != string::npos && pos2 != string::npos)
            log("[Enet Faild] 天翼校园网登陆失败！- " + strResponse.substr(pos1, pos2 - pos1) + "。");
        else
            log("[Enet Faild] 天翼校园网登陆失败！- 原因未知。");
    }
    //free(password1);
    return result;
}


bool TProcess::runDetectActive()
{
    bool reset = false, retry = false;
    time_t retryTime = -1;
    int cnt = 0;
    suspend(30);
    while(1)
    {
        suspend(1);
        cnt++; if(cnt >= 3600) cnt %= 3600;
        if(!reset && cnt >= 300) reset = true; // 程序开始运行5分钟内不做检测
        if(dpcap)
        {
            // 如果认证服务器在一定时间没有与软件交互，则重新发包
            if(timeout != 0 && reset && (cnt % 10 == 0) && dpcap->getRunStatus() &&
                 (dpcap->getPreRVal() >= timeout) && !runHeartbean(true)) // 超时时间默认0秒，可自定义
            {
                log("[Reset] 网络错误,重置中兴认证...");
                dpcap->setStatusAndRun(DPcap::EapType::EAPOL_START);
            }

            // 认证错误的情况
            if(!retry && (retryTime = dpcap->getReSendTime()) != -1) retry = true;
            if(retry && retryTime <= time(0))
            {
                //log("[Debug] 正在重试发包...");
                dpcap->setStatusAndRun(DPcap::EapType::EAPOL_START);
                retry = false;
            }

        }
    }
}

DPcap* TProcess::getDPcapHandle()
{
    return dpcap;
}

// 日志记录，实际上直接调用DLog类
// logFile和mutex_log在本类头文件中定义，已注释掉
bool TProcess::log(const string & str)
{
    if(dlog) return dlog->log(str);
    else return false;
    /*
    fstream fout;
    time_t t = time(0);
    char timestr[64];

    strftime(timestr, sizeof(timestr), "[%m/%d %H:%M:%S]", localtime(&t));
    pthread_mutex_lock(&mutex_log);
    fout.open(logFile, std::ios_base::out | std::ios_base::app);
    if(!fout.is_open())
    {
        pthread_mutex_unlock(&mutex_log);
        return false;
    }
    std::cout << timestr << " " << str << std::endl;
    fout << timestr << " " << str << std::endl;
    fout.close();
    pthread_mutex_unlock(&mutex_log);
    return true;
    */
}


// 为了日后兼容win和linux两个版本设置了个睡眠的函数，
// 在linux下此函数和sleep效果相似
bool TProcess::suspend(int s)
{
    if(s < 0) return true;
#ifndef WIN32
    ::sleep(s);
#else
    ::Sleep(s * 1000);
#endif
    return true;
}
