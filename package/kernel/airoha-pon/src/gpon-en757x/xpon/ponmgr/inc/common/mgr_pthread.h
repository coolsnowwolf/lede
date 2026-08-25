#ifndef _MGR_PTHREAD_H_
#define _MGR_PTHREAD_H_

#include <pthread.h>

#define PTHREAD_CREATE_JOIN			NULL
#define PTHREAD_CREATE_DETACH		&mgrThreadAttr

#define PONMGR_PTHREAD_CREATE(thread_handler, arg, mode)	pthread_create(&mgrThread, mode, thread_handler, arg)
#define PONMGR_PTHREAD_JOIN(thread_return)					pthread_join(mgrThread, thread_return)
#define PONMGR_PTHREAD_EXIT(retval)							pthread_exit(retval)

extern pthread_t mgrThread ;
extern pthread_attr_t mgrThreadAttr ;

#endif /* _MGR_PTHREAD_H_ */
