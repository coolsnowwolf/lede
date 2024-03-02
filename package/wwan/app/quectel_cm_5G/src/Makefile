ifneq ($(CROSS_COMPILE),)
CROSS-COMPILE:=$(CROSS_COMPILE)
endif
#CROSS-COMPILE:=/workspace/buildroot/buildroot-qemu_mips_malta_defconfig/output/host/usr/bin/mips-buildroot-linux-uclibc-
#CROSS-COMPILE:=/workspace/buildroot/buildroot-qemu_arm_vexpress_defconfig/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-
#CROSS-COMPILE:=/workspace/buildroot-git/qemu_mips64_malta/output/host/usr/bin/mips-gnu-linux-
ifeq ($(CC),cc)
CC:=$(CROSS-COMPILE)gcc
endif
LD:=$(CROSS-COMPILE)ld

QL_CM_SRC=QmiWwanCM.c GobiNetCM.c main.c QCQMUX.c QMIThread.c util.c qmap_bridge_mode.c mbim-cm.c device.c
QL_CM_SRC+=atc.c atchannel.c at_tok.c
#QL_CM_SRC+=qrtr.c rmnetctl.c
ifeq (1,1)
QL_CM_DHCP=udhcpc.c
else
LIBMNL=libmnl/ifutils.c libmnl/attr.c libmnl/callback.c libmnl/nlmsg.c libmnl/socket.c
DHCP=libmnl/dhcp/dhcpclient.c libmnl/dhcp/dhcpmsg.c libmnl/dhcp/packet.c
QL_CM_DHCP=udhcpc_netlink.c
QL_CM_DHCP+=${LIBMNL}
endif

CFLAGS += -Wall -Wextra -Werror -O1 #-s
LDFLAGS += -lpthread -ldl -lrt

release: clean qmi-proxy mbim-proxy atc-proxy #qrtr-proxy
	$(CC) ${CFLAGS} ${QL_CM_SRC} ${QL_CM_DHCP} -o out/quectel-CM ${LDFLAGS}

debug: clean
	$(CC) ${CFLAGS} -g -DCM_DEBUG ${QL_CM_SRC} ${QL_CM_DHCP} -o out/quectel-CM -lpthread -ldl -lrt

qmi-proxy:
	$(CC) ${CFLAGS} quectel-qmi-proxy.c -o out/quectel-qmi-proxy ${LDFLAGS} 

mbim-proxy:
	$(CC) ${CFLAGS} quectel-mbim-proxy.c -o out/quectel-mbim-proxy ${LDFLAGS} 

qrtr-proxy:
	$(CC) ${CFLAGS} quectel-qrtr-proxy.c -o out/quectel-qrtr-proxy ${LDFLAGS} 

atc-proxy:
	$(CC) ${CFLAGS} quectel-atc-proxy.c atchannel.c at_tok.c util.c -o out/quectel-atc-proxy ${LDFLAGS} 

clean:
	cd out/; rm -rf *.o libmnl/*.o quectel-CM quectel-qmi-proxy quectel-mbim-proxy quectel-atc-proxy
