#ifndef _MGR_SEM_H_
#define _MGR_SEM_H_

#include <semaphore.h>

#define MARCO_SEM_LOCK(arg)					sem_wait(arg)
#define MARCO_SEM_UNLOCK(arg)				sem_post(arg)

#define SEM_LOCK				0
#define SEM_UNLOCK				1

extern sem_t semAppInfo ;			//declared in mmgr_init.c


#endif /* _MGR_SEM_H_ */
