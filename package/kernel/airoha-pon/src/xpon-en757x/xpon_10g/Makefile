KERNELDIR=$(KERNEL_DIR)
EXTRA_CFLAGS += -DWAN2LAN

INCDIR		:= inc
SRCDIR		:= src

ifneq ($(strip $(TCSUPPORT_OPENWRT)),)
include $(OPENWRT_BUILD_ROOT)/BSP/xpon_bsp/xpon_dir.mak
else
ifneq ($(strip $(RDKB_BUILD)),)
ifneq ($(strip $(ECNT_SDK_ROOT)),)
include $(ECNT_SDK_ROOT)/BSP/xpon_bsp/xpon_dir.mak
endif
endif
endif

EXTRA_CFLAGS += -I$(XPON_BSP_MODULES_XPON_10G_DRIVER)/$(INCDIR) -I$(XPON_BSP_MODULES_XPON_10G_DRIVER)/$(INCDIR)/epon  -I$(XPON_BSP_GLOBAL_INC_PRIVATE) \
			-I$(XPON_BSP_GLOBAL_INC_PUBLIC) -I$(XPON_BSP_ECNT_HOOK_DIR)/include

ifeq ($(strip $(TCSUPPORT_3_18_21_KERNEL) $(TCSUPPORT_4_4_KERNEL)),)
EXTRA_CFLAGS += -O0 
#-save-temps
endif
EXTRA_CFLAGS += -D CONFIG_SEND_ACK_PLOAMU=3 -D XPON_MAC_CONFIG_DEBUG \
				-D CONFIG_USE_FOR_TEST \
				-D XPON_MAC_CONFIG_DEBUG 

ifneq ($(TCSUPPORT_CPU_EN7521),)
	EXTRA_CFLAGS += -D CONFIG_USE_RX_NAPI
endif


EXTRA_CFLAGS += -DEPON_MAC_HW_TEST
EXTRA_CFLAGS += -DTCSUPPORT_EPON_RGST_SILENT
EXTRA_CFLAGS += -D TCSUPPORT_EPON_POWERSAVING

#connect wan for xpon_10g
EXTRA_CFLAGS += -D CONFIG_CONNECT_WAN_DEBUG 

####-Warning for error MUST at last of EXTRA_CFLAGS
EXTRA_CFLAGS+=-Werror -Wno-error=frame-larger-than=

ifneq ($(TCSUPPORT_WAN_GPON),)
	GPON_SRC	:= gpon_init.c gpon_ploam.c gpon.c gpon_dev.c gpon_act.c gpon_dvt.c gpon_proc.c gpon_security.c gpon_recovery.c
	GPON_OBJ	:= $(GPON_SRC:%.c=$(SRCDIR)/gpon/%.o)

	XMCS_SRC	+= xmcs_gpon.c 
	PWAN_SRC	+= gpon_wan.c 
endif

ifneq ($(TCSUPPORT_WAN_EPON),)
	EPON_SRC	:= epon_debug.c epon_init.c epon_timer.c epon_act.c epon_ioctl.c epon_mpcp.c epon_isr.c epon_phy_event.c epon_compile_option_wrapper.c epon_dev.c epon_msg_route.c
	EPON_OBJ	:= $(EPON_SRC:%.c=$(SRCDIR)/epon/%.o)

	XMCS_SRC 	+= xmcs_epon.c 
	PWAN_SRC	+= epon_wan.c
endif

ifneq ($(TCSUPPORT_CPU_EN7581),)
IC_SRC		:= AN7581.c
else
IC_SRC		:= EN7580.c
endif

IC_OBJ		:= $(IC_SRC:%.c=$(SRCDIR)/ic/%.o)

XMCS_SRC	+= xmcs_mci.c xmcs_if.c xmcs_fdet.c xmcs_phy.c
XMCS_OBJ	:= $(XMCS_SRC:%.c=$(SRCDIR)/xmcs/%.o)

PWAN_SRC	+= xpon_netif.c 
PWAN_OBJ	:= $(PWAN_SRC:%.c=$(SRCDIR)/pwan/%.o)

TOTAL_OBJ	:= $(SRCDIR)/xpondrv.o $(SRCDIR)/phy_if_wrapper.o $(SRCDIR)/xpon_led.o $(SRCDIR)/xpon_daemon.o $(SRCDIR)/omci_oam_monitor.o $(SRCDIR)/xpon_api.o $(SRCDIR)/union_ic_init.o\
       		   $(GPON_OBJ) $(EPON_OBJ) $(PWAN_OBJ) $(XMCS_OBJ) $(IC_OBJ)


xpon_10g-objs 	:= $(TOTAL_OBJ)
obj-m		:= xpon_10g.o

all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules
	$(STRIP) --strip-unneeded xpon_10g.ko
	
clean:
	# cd $(FILESYSTEM_DIR)/lib/modules/ && rm -f xpon_10g.ko
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean

