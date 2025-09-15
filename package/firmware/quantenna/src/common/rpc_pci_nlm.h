#ifndef __PCI_NLM_H__

#define __PCI_NLM_H__

/*
 * We seperate the netlink type for client and server here.
 * If the netlink type is conflicted with customers', they just need to modify
 * NETLINK_RPC_PCI_CLNT and the type define in the PCIe RC driver and the netlink
 * type in the rpc server and PCIe EP driver will not be affected.
 */
#define NETLINK_RPC_PCI_CLNT 31
#define NETLINK_RPC_PCI_SVC 31
#define PCIMSGSIZE (64 * 1024 - 1)

/*
 * Nelink Message types.
 */
#define RPC_TYPE_CALL_QCSAPI_PCIE	0x0100
#define RPC_TYPE_QCSAPI_PCIE		0x0200

#define NETLINK_TYPE_SVC_REGISTER	(PCIE_RPC_TYPE | 0x0010)
#define NETLINK_TYPE_SVC_RESPONSE	(PCIE_RPC_TYPE | 0x0011)
#define NETLINK_TYPE_CLNT_REGISTER	(PCIE_RPC_TYPE | 0x0010)
#define NETLINK_TYPE_CLNT_REQUEST	(PCIE_RPC_TYPE | 0x0011)

#endif
