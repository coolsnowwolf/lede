#ifndef DLOG_H_INCLUDED
#define DLOG_H_INCLUDED

#include <fstream>
#include <pthread.h>

class DLog
{
public:
    DLog(const std::string & file = "/tmp/dlog.txt");
    ~DLog();
    bool log(const std::string & str);
    //bool clean();
    //int getSize();
private:
    std::fstream fout;
    std::string logFileName;
    pthread_mutex_t mutex_log;
};

#endif // DLOG_H_INCLUDED
