#include <stdio.h>
#include <errno.h>

#include "common/mgr_ipc.h"

/*****************************************************************************
*****************************************************************************/
int ipc_msg_queue_create(key_t key)
{
	int msgid;
	
	if((msgid = msgget(key, IPC_CREAT|0777)) < 0) {
		return -1 ;
	}
	
	return msgid ;	
}


/*****************************************************************************
*****************************************************************************/
int ipc_msg_queue_get_id(key_t key)
{
	int msgid;
	
	if((msgid = msgget(key, 0777)) < 0) {
		return -1 ;
	}
	
	return msgid ;	
}


/*****************************************************************************
*****************************************************************************/
int ipc_msg_queue_delete(key_t key, int msgid)
{

	if(key != 0) {
		msgid = msgget(key , 0);
		
		if(msgid < 0) {
			return -1 ;		
		} 
	}
	
	if(msgctl(msgid, IPC_RMID, NULL) <0) {
		return -1 ;
	}
	
	return 0 ;
}

/*****************************************************************************
*****************************************************************************/
int ipc_msg_queue_send(int msgid, IPC_Msg_T *pMsg)
{
	if((msgsnd(msgid, pMsg, sizeof(MGR_CmdMsg_T), 0)) < 0) {
		return -1 ;
	}
	
	return 0 ;
}

/*****************************************************************************
*****************************************************************************/
int ipc_msg_queue_recv(int msgid, long type, IPC_Msg_T *pMsg)
{
	int len ;
	
	len = msgrcv(msgid, pMsg, sizeof(MGR_CmdMsg_T), type,0) ;
	
	if(len <= 0) {
		return -1 ;
	}
	
	return len ;
}

