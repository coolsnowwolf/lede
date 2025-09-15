# 
# Makefile for Octeon platform
#

ifndef OCTEON_ROOT
OCTEON_ROOT 	:= ../..
endif

include $(OCTEON_ROOT)/common.mk

COMMON_DIR	:= ../common
MAIN_INCLUDES	:= $(PWD)/../../include
INCLUDES	:= -I$(COMMON_DIR) -I$(PWD) -I$(MAIN_INCLUDES)

CAVIUM_INCLUDES	:= -I ${OCTEON_ROOT}/target/include $(INCLUDES)
 
KERNEL_PATH	:= ../../../../linux/kernel_2.6/linux
kernel_source 	:= $(KERNEL_PATH) 
CROSS_COMPILE 	:= mips64-octeon-linux-gnu-

CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld

# Common flags to be passed for driver compilation
EXTRA_CFLAGS	+= -Winline -Wall -I arch/mips/cavium-octeon/gpl-executive/config ${CAVIUM_INCLUDES}
EXTRA_CFLAGS	+= -I $(srctree)/arch/mips/cavium-octeon/gpl-executive/config -I ${OCTEON_ROOT}/executive
EXTRA_CFLAGS	+= -I drivers/net/cavium-ethernet

EXTRA_CFLAGS	+= -DQDPC_CVM_DMA
EXTRA_CFLAGS	+= -DQTN_PCIE_USE_LOCAL_BUFFER
#EXTRA_CFLAGS	+= -DDEBUG

default: all
qdpc-host-objs	:= qdpc_cvm_dma.o \
                    qdpc_platform.o \
                    $(COMMON_DIR)/qdpc_ring.o \
                    $(COMMON_DIR)/qdpc_net.o  \
                    $(COMMON_DIR)/qdpc_pcie.o \
                    $(COMMON_DIR)/qdpc_init.o

obj-m		:=  qdpc-host.o 

all:
	$(MAKE) -C $(kernel_source) SUBDIRS=`pwd`  modules;

clean:
	rm -rf *.o  *~  core  .depend  .*.cmd   *.ko   *.mod.c  .tmp_versions  Module.symvers 
	rm -rf $(COMMON_DIR)/.*.cmd $(COMMON_DIR)/.tmp_versions 
	rm -rf Module.markers  modules.order $(qdpc-host-objs)

