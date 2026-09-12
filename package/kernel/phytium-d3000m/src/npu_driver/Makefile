# SPDX-License-Identifier: GPL-2.0

M ?= $(CURDIR)
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

obj-m += phytium_npu.o
phytium_npu-objs := phytium-npu/phytium_npu_common.o \
	phytium-npu/phytium_npu_dev.o \
	phytium-npu/phytium_npu_dev_mmu.o \
	phytium-npu/phytium_npu_mm_mmu.o \
	phytium-npu/phytium_npu_workqueue.o \
	phytium-npu/phytium_npu_session.o \
	phytium-npu/phytium_npu_uapi.o \
	phytium-npu/phytium_npu_debug.o

obj-m += phytium_npu_platform.o
phytium_npu_platform-objs := phytium-npu/phytium_npu_platform.o

obj-m += phytium_npu_pci.o
phytium_npu_pci-objs := phytium-npu/phytium_npu_pci.o

BUILD_HEAP ?= n
ifeq ($(BUILD_HEAP),y)
obj-m += phytium_npu_heap.o
phytium_npu_heap-objs := heap/phytium_npu_heap.o
endif

ccflags-y += -I$(M)/phytium-npu -I$(M)/include

all modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
