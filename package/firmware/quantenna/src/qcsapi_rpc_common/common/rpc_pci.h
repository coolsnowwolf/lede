#ifndef _PCI_RPC_H
#define _PCI_RPC_H

#include "rpc_pci_nlm.h"

extern CLIENT *
clnt_pci_create (const char *hostname,
		u_long prog,
		u_long vers,
		const char *proto);

extern SVCXPRT *svc_pci_create (int sock);

#endif
