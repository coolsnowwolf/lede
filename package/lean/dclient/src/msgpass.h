#ifndef MSIGNAL_H_INCLUDED
#define MSIGNAL_H_INCLUDED

#include <pthread.h>

class MsgPass
{
public:
    MsgPass();
    ~MsgPass();
    void signal();
    void wait(void* (*fun)(void *), void * data);
protected:
    pthread_cond_t cond;
    pthread_mutex_t mutex;
};

#endif // MSIGNAL_H_INCLUDED
