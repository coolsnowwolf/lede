bin_PROGRAMS = quectel-CM
QL_CM_SRC=QmiWwanCM.c GobiNetCM.c main.c MPQMUX.c QMIThread.c util.c qmap_bridge_mode.c mbim-cm.c device.c
QL_CM_SRC+=atc.c atchannel.c at_tok.c
#QL_CM_SRC+=qrtr.c rmnetctl.c
QL_CM_DHCP=udhcpc.c
if USE_QRTR
quectel_CM_CFLAGS = -DCONFIG_QRTR 
QL_CM_SRC += qrtr.c rmnetctl.c
if USE_MSM_IPC
quectel_CM_CFLAGS += -DUSE_LINUX_MSM_IPC
endif
endif

quectel_CM_SOURCES = ${QL_CM_SRC} ${QL_CM_DHCP} 

bin_PROGRAMS += quectel-qmi-proxy
quectel_qmi_proxy_SOURCES = quectel-qmi-proxy.c

bin_PROGRAMS += quectel-mbim-proxy
quectel_mbim_proxy_SOURCES = quectel-mbim-proxy.c
LIBS = -l pthread
CFLAGS = -Wall -Wextra -Werror -O1
