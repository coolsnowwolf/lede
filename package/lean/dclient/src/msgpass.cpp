#include "msgpass.h"

MsgPass::MsgPass()
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

MsgPass::~MsgPass()
{
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

void MsgPass::signal()
{
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void MsgPass::wait(void* (*fun)(void *), void * data)
{
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    fun(data);
    pthread_mutex_unlock(&mutex);
}
