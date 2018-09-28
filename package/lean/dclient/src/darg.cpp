#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "darg.h"
#include <getopt.h>
#include <cstdlib>
#include "lock.h"
#include "daemon.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

string Version = "2017.09.19.20";


DArg::DArg()
{
    m_zteUser = m_ztePass = m_adapterName = m_enetUser = m_enetPass = msg = "";
    background = 0;
    m_dhcpStyle = 0; // 默认不使用dhcp
    m_timeout = 0; // 默认不设置超时
}

DArg::~DArg()
{

}

void DArg::showUsage()
{
    cout << "\n\t\tZTE 802.1X Client For CCDGUT (Version: " << ::Version << ")\n\n";
    cout << "\tUsage:\n\n";
    cout << "Required arguments:\n"
            "\t-u, --zteuser\t\tYour username.\n"
            "\t-p, --ztepass\t\tYour password.\n"
            "\t-d, --device\t\tSpecify which device to use.\n"
            "\n"
            "Optional arguments:\n"
            "\t-e, --enetUser\t\tEnet auth username\n"
            "\t-k, --enetPass\t\tEnet auth password\n"
            "\t-i, --DhcpClient\tSelect DhcpClient\n"
            "\t\tonly support dhclient and udhcpc, default is disable\n"
            "\t-s, --dhcp script file\n"
            "\t-b, --daemon\t\tRun as daemon\n"
            "\t-t, --authentication timeout\n"
            //"\t-r, --reconnect\t\tReconnect\n"
            //"\t-l, --logoff\t\tLogoff.\n"
            "\t-h, --help\t\tShow this help.\n"
            "Note: Run under root privilege, usually by 'sudo'.\n"
            "\n"
            "\t\t\t\t\t\t\t\tBy Zeyes\n";
}


void DArg::getArguments(int argc, char *argv[])
{
int background = 0, c;


    if(argc == 1)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    const char* short_options = "u:p:d:t:be:k:i:s:rlh";
    struct option long_options[] =
    {
        {"zteuser", required_argument, NULL, 'u'},
        {"ztepass", required_argument, NULL, 'p'},
        {"device",  required_argument, NULL, 'd'},
        {"timeout", required_argument, NULL, 't'},
        {"daemon", no_argument, no_argument, 'b'},
        {"enetuser", required_argument, NULL, 'e'},
        {"enetpass", required_argument, NULL, 'k'},
        {"reconnect", no_argument, NULL, 'r'},
        {"logoff", no_argument, NULL, 'l'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };

    while((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1)
    {
        switch(c)
        {
        case 'u':
            m_zteUser = optarg;
            break;
        case 'p':
            m_ztePass = optarg;
            break;
        case 'd':
            m_adapterName = optarg;
            break;
        case 't':
            m_timeout = atoi(optarg);
            break;
        case 'b':
            background = 1;
            break;
        case 'e':
            m_enetUser = optarg;
            break;
        case 'k':
            m_enetPass = optarg;
            break;
        case 'i':
            if (!strcmp(optarg, "dhclient"))
                m_dhcpStyle = 1;
            else if (!strcmp(optarg, "udhcpc"))
                m_dhcpStyle = 2;
            else
                cerr << "Unknow dhcp client:" << optarg << ", use dhclient instead.\n";
            break;
        case 's':
            m_dhcpScript = optarg;
            break;
        case 'h':
            showUsage();
            exit(EXIT_SUCCESS);
            break;
        case 'r':
            break;
        case 'l':
            break;
        case '?':
            exit(EXIT_FAILURE);
        default:
            cerr << "Unrecognize option -" << optopt << "\n";
        }
    }

    if(background == 1)
    {
        cout << "Running as daemon!...\n";
        init_daemon();  // 守护进程
    }

    if(already_running())
    {
        cerr << "Program is running!\n";
        exit(EXIT_FAILURE);
    }


    if(m_zteUser.empty() || m_ztePass.empty() || m_adapterName.empty())
    {
        cerr << "--zteuser, --ztepass, --device are vital.\n";
        exit(EXIT_FAILURE);
    }

    if(m_timeout != 0 && m_timeout < 300)
    {
        cerr << "timeout value can't set, because the minimum value is 300.\n";
        m_timeout = 300;
    }

    if(m_enetUser.empty()) m_enetAuth = false; else m_enetAuth = true;
}


string DArg::zteUser()
{
    return m_zteUser;
}
string DArg::ztePass()
{
    return m_ztePass;
}
string DArg::adapterName()
{
    return m_adapterName;
}
string DArg::enetUser()
{
    return m_enetUser;
}
string DArg::enetPass()
{
    return m_enetPass;
}
int DArg::dhcpStyle()
{
    // 0is disable, 1 is dhclient, 2 is udhcpc
    return m_dhcpStyle;
}
string DArg::dhcpScript()
{
    return m_dhcpScript;
}
string DArg::getMessage()
{
    return msg;
}

bool DArg::enetAuth()
{
    return m_enetAuth;
}

int DArg::timeout()
{
    return m_timeout;
}
