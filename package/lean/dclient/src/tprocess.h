#ifndef TPROCESS_H_
#define TPROCESS_H_

#include <ctime>
#include <fstream>
#include <pthread.h>
#include "dpcap.h"
#include "adapter.h"
//#include "estatus.h"
#include "dcurl.h"
#include "msgpass.h"
#include "dlog.h"

using std::string;
using std::fstream;


class TProcess
{
public:
    TProcess(const string & zteUser, const string & ztePass, const string & adapterName = "eth0",
             int dhcpStyle = 0, string dhcpScript="", int timeout = 0, int enetAuth = 0, const string & enetUser = "",
             const string & enetPass = "");
    ~TProcess();
    void runDPcap();
    void runDPcapWait();
    DPcap* getDPcapHandle();
    void runDCurl();
    bool runLoginEnet(const unsigned char * ip);
    bool runHeartbean(bool toVar, bool local = false);
    bool runDetectActive();
    int run();
    bool log(const string & str);
protected:
    bool suspend(int s);
private:
    string zteUser, ztePass, enetUser, enetPass, adapterName, dhcpScript;
    int dhcpStyle, timeout, enetAuth, change;
    //pthread_mutex_t mutex_log;
    //string logFile;
    DPcap * dpcap;
    DCurl * dcurl;
    DLog  * dlog;
    MsgPass * msgPass;
};
#endif // TPROCESS_H_
