#include <iostream>
#include <ctime>
#include "dlog.h"

DLog::DLog(const std::string & file)
{
    pthread_mutex_init(&mutex_log, NULL);
    logFileName = file;
    //fout.open(logFileName, std::ios_base::out | std::ios_base::app);
}

DLog::~DLog()
{
    pthread_mutex_destroy(&mutex_log);
    //fout.close();
}

bool DLog::log(const std::string & str)
{
    time_t t = time(0);
    char timestr[64];

    strftime(timestr, sizeof(timestr), "[%m/%d %H:%M:%S] ", localtime(&t));

    pthread_mutex_lock(&mutex_log);
    fout.open(logFileName, std::ios_base::out | std::ios_base::app);
    if(!fout.is_open())
    {
        pthread_mutex_unlock(&mutex_log);
        return false;
    }
    std::cout << timestr << " " << str << std::endl << std::flush;
    fout << timestr << " " << str << std::endl << std::flush;
    fout.close();
    pthread_mutex_unlock(&mutex_log);
    return true;
}
