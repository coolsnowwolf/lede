#ifndef OMCI_DBG_SRV_H_
#define OMCI_DBG_SRV_H_

#include "omci_external_msg.h"
#include "omci_types.h"
#define GPON_OMCI_DBG_PORT   8809
#define GPON_OMCI_DBG_MAX_BUF  2000

int tcp_server_setup(uint16 port);
int tcp_server_exit();
int accept_client();

#define GET_DATA_FROM_OLT              0
#define GET_DATA_FROM_DBG_QUEUE  1

int gponOmciPktRxDbgQ(omciPayload_t *omciPkt);
int gponOmciPktTxDbgQ(int len, char *pkts);
void *gponOmciDbgThread(void *data);
#endif
