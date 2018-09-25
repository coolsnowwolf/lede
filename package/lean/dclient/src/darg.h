#ifndef DARG_H_
#define DARG_H_

#include <string>

using std::string;

class DArg
{
public:
    DArg();
    ~DArg();
    void showUsage();
    void getArguments(int argc, char *argv[]);
    string zteUser();
    string ztePass();
    string adapterName();
    string enetUser();
    string enetPass();
    int dhcpStyle();
    string dhcpScript();
    int timeout();
    bool enetAuth();
    //void logoff();
    //bool daemon();
    string getMessage();
protected:
    string m_zteUser, m_ztePass, m_adapterName, m_enetUser, m_enetPass, m_dhcpScript;
    string msg;
    int background;
    int m_dhcpStyle, m_timeout;
    bool m_enetAuth;
};

#endif // DARG_H_
