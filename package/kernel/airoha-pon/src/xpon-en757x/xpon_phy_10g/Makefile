KERNELDIR=$(KERNEL_DIR)
EXTRA_CFLAGS += -DWAN2LAN

INCDIR		:= inc
SRCDIR		:= src
HEADER_DIR              := inc

ifneq ($(TCSUPPORT_CPU_EN7521),)
	EXTRA_CFLAGS += -I$(MODULES_QDMA_7512_DIR)
else
	EXTRA_CFLAGS += -I$(MODULES_QDMA_DIR)
endif

EXTRA_CFLAGS += -I$(MODULES_PHY_10G_DIR)/$(HEADER_DIR) -O0 
EXTRA_CFLAGS += -D CONFIG_SEND_ACK_PLOAMU=3 \
				-D CONFIG_USE_FOR_TEST \
				-D CONFIG_DEBUG 

ifeq ($(CONFIG_USE_MT7520_ASIC),)
#EXTRA_CFLAGS += -DCONFIG_USE_A60901
#EXTRA_CFLAGS += -DCONFIG_USE_A60928
endif
				
EXTRA_CFLAGS += -DEPON_MAC_HW_TEST
EXTRA_CFLAGS += -DTCSUPPORT_EPON_RGST_SILENT

ifneq ($(TCSUPPORT_EPON_FPGA_WORKAROUND),)
EXTRA_CFLAGS += -DTCSUPPORT_EPON_FPGA_WORKAROUND
endif

ifneq ($(TCSUPPORT_CPU_AN7583),)
	PHY_SRC	+= i2c.c phy.c phy_init.c phy_rx.c phy_tx.c en7583.c a60972.c 
	PHY_OBJ	:= $(PHY_SRC:%.c=$(SRCDIR)/%.o)
else	
ifneq ($(TCSUPPORT_CPU_EN7581),)
	PHY_SRC	+= i2c.c phy.c phy_init.c phy_rx.c phy_tx.c en7581.c a60972.c en7581_pma.c
	PHY_OBJ	:= $(PHY_SRC:%.c=$(SRCDIR)/%.o)
else
	PHY_SRC	+= i2c.c phy.c phy_init.c phy_rx.c phy_tx.c en7580.c a60972.c en7580_pma.c
	PHY_OBJ	:= $(PHY_SRC:%.c=$(SRCDIR)/%.o)
endif
endif

TOTAL_OBJ	:= $(SRCDIR)/phy.o  $(PHY_OBJ) 

phy_10g-objs 	:= $(TOTAL_OBJ)
obj-m		:= phy_10g.o

all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules
	$(STRIP) --strip-unneeded phy_10g.ko
	
clean:
	# cd $(FILESYSTEM_DIR)/lib/modules/ && rm -f phy_10g.ko
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean


