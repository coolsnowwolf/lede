#ifndef _MGR_IPC_H_
#define _MGR_IPC_H_

#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include "common/mgr_config.h"
#include "common/mgr_struct.h"


#define IPC_MSG_QUEUE_OMCI					988
#define IPC_MSG_QUEUE_OAM					8023

typedef enum {
	ENUM_MSG_TYPE_UNKNOWN = 0,
	ENUM_MSG_TYPE_CMD,
	ENUM_MSG_TYPE_TRAP
} MGR_MsgType_t ;


typedef struct {
	long 			mtype ;	//ENUM_MSG_TYPE_CMD, ENUM_MSG_TYPE_TRAP
	MGR_CmdMsg_T 	data ;
} IPC_Msg_T ;


extern MGR_S32 gOmciTrapQueueId ;
extern MGR_S32 gOamTrapQueueId ;


/******************************************************************
 Prototype for IPC Message Queue
******************************************************************/
int ipc_msg_queue_create(key_t key) ;
int ipc_msg_queue_get_id(key_t key) ;
int ipc_msg_queue_delete(key_t key, int msgid) ;
int ipc_msg_queue_send(int msgid, IPC_Msg_T *pMsg) ;
int ipc_msg_queue_recv(int msgid, long type, IPC_Msg_T *pMsg) ;


#endif /* _MGR_IPC_H_ */

