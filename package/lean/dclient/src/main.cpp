#include <iostream>
#include <string>

#include "tprocess.h"
#include "darg.h"

using namespace std;


string zteUser, ztePass, enetUser, enetPass, adapterName, dhcpScript;
bool enetAuth;
int dhcpStyle, timeout;

void runProcess();

int main(int argc, char *argv[])
{
    DArg *darg = new DArg();
    darg->getArguments(argc, argv);
    //zteUser  = "201535020421";
    //ztePass  = "123456";
    //enetUser = "201535020421";
    //enetPass = "12345678";
    //adapterName = "eth0.2";
    zteUser  = darg->zteUser();
    ztePass  = darg->ztePass();
    adapterName = darg->adapterName();
    enetAuth = darg->enetAuth();
    dhcpStyle = darg->dhcpStyle();
    dhcpScript = darg->dhcpScript();
    timeout = darg->timeout();
    if(enetAuth)
    {
        enetUser = darg->enetUser();
        enetPass = darg->enetPass();
    }

    runProcess();
    return 0;
}

void runProcess()
{
    TProcess * tp = new TProcess(zteUser, ztePass, adapterName, dhcpStyle, dhcpScript, timeout, enetAuth, enetUser, enetPass);
    printf("pthread exit with %d\n", tp->run());
}
