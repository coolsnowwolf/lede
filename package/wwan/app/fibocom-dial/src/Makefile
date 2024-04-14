ifneq ($(CROSS_COMPILE),)
CROSS-COMPILE:=$(CROSS_COMPILE)
endif

CFLAGS += -DGHT_FEATURE_PCIE_AUTO

ifeq ($(CC),cc)
CC:=$(CROSS-COMPILE)gcc
endif
LD:=$(CROSS-COMPILE)ld

SRC=QmiWwanCM.c  GobiNetCM.c main.c MPQMUX.c QMIThread.c util.c qmap_bridge_mode.c query_pcie_mode.c

FB_DHCP=udhcpc.c

FIBO_PROXY_SRC=fibo_qmimsg_server.c

LIBMNL=libmnl/ifutils.c libmnl/attr.c libmnl/callback.c libmnl/nlmsg.c libmnl/socket.c
FB_NDHCP=udhcpc_netlink.c
FB_NDHCP+=${LIBMNL}

release: clean
	$(CC) $(CFLAGS) -Wall -s ${SRC} ${FB_NDHCP} -o fibocom-dial -lpthread -ldl
	$(CC) -Wall -s  multi-pdn-manager.c  query_pcie_mode.c util.c -o multi-pdn-manager -lpthread -ldl
	$(CC) -Wall -s ${FIBO_PROXY_SRC} -o fibo_qmimsg_server -lpthread -ldl

dhcp: clean
	$(CC) $(CFLAGS) -Wall -s ${SRC} ${FB_DHCP} -o fibocom-dial -lpthread -ldl
	$(CC) -Wall -s  multi-pdn-manager.c  query_pcie_mode.c util.c -o multi-pdn-manager -lpthread -ldl
	$(CC) -Wall -s ${FIBO_PROXY_SRC} -o fibo_qmimsg_server -lpthread -ldl

ndhcp: clean
	$(CC) $(CFLAGS) -Wall -s ${SRC} ${FB_NDHCP} -o fibocom-dial -lpthread -ldl
	$(CC) -Wall -s  multi-pdn-manager.c   query_pcie_mode.cutil.c -o multi-pdn-manager -lpthread -ldl
	$(CC) -Wall -s ${FIBO_PROXY_SRC} -o fibo_qmimsg_server -lpthread -ldl

qmi-proxy:
	$(CC) -Wall -s fibo-qmi-proxy.c  -o fibo-qmi-proxy -lpthread -ldl

clean:
	rm -rf  fibocom-dial *~ multi-pdn-manager fibo_qmimsg_server

