KERNELDIR=$(KERNEL_DIR)
EXTRA_CFLAGS += -DWAN2LAN

INCDIR		:= inc
SRCDIR		:= src

EXTRA_CFLAGS += -I$(XPON_BSP_MODULES_XPON_10G_DRIVER)/$(INCDIR) -I$(XPON_BSP_MODULES_XPON_10G_DRIVER)/$(INCDIR)/common -I$(MODULES_XPON_10G_CUSTOMER_FH_DIR)/$(INCDIR) -I$(XPON_BSP_GLOBAL_INC_PRIVATE) \
			-I$(XPON_BSP_GLOBAL_INC_PUBLIC) 
ifeq ($(strip $(TCSUPPORT_3_18_21_KERNEL)),)
EXTRA_CFLAGS += -O0 -save-temps
endif

####-Warning for error MUST at last of EXTRA_CFLAGS
EXTRA_CFLAGS+=-Werror

CUSTOMER_SRC	:= xpon_fh.c
CUSTOMER_OBJ	:= $(CUSTOMER_SRC:%.c=$(SRCDIR)/%.o)

xpon_custom_fh-objs 	:= $(CUSTOMER_OBJ)
obj-m		:= xpon_custom_fh.o

all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules
	$(STRIP) --strip-unneeded xpon_custom_fh.ko
	
clean:
	# rm -f $(FILESYSTEM_DIR)/lib/modules/xpon_custom_fh.ko
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean

