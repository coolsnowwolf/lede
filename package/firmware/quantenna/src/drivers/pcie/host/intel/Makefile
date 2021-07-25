#
# Makefile for Intel platform
#

COMMON_DIR	:= ../common
MAIN_INCLUDES	:= $(PWD)/../../include
INCLUDES	:= -I$(COMMON_DIR) -I$(PWD) -I$(MAIN_INCLUDES)

EXTRA_CFLAGS	+= -Winline -Wall -O2 $(INCLUDES)
EXTRA_CFLAGS	+= -DQTN_PCIE_USE_LOCAL_BUFFER
#EXTRA_CFLAGS	+= -DDEBUG

KVERSION = $(shell uname -r)

default: all

qdpc-host-objs 	:=	$(COMMON_DIR)/qdpc_ring.o \
			$(COMMON_DIR)/qdpc_net.o  \
			$(COMMON_DIR)/qdpc_pcie.o \
			$(COMMON_DIR)/qdpc_init.o

obj-m 		:= 	qdpc-host.o

qdpc_host.o: $(qdpc-host-objs)
	ld -r $^ -o $@

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
	rm -rf $(COMMON_DIR)/.*.cmd $(COMMON_DIR)/.tmp_versions
	rm -rf Module.markers  modules.order *~ $(qdpc-host-objs)

