# Customer poring maybe need replace following macro
# PCIE_EN and PHY_FPGA_PCIE
# PHY_FPGA_MEM
# USE_CMA

include $(PWD)/vpu_driver/ipoffset/project_config.mk
#include $(PWD)/ipoffset/project_config.mk

obj-m := ftv310_vpu_driver.o
ftv310_vpu_driver-objs = ftv310_vpu_drv.o ftv310_vpu_fence.o \
	vcmdswhwregisters.o bidirect_list.o devicemgr.o ftv310_vpu_dmabuf.o ftv310_vpu_fs.o ftv310_vpu_drm.o ftv310_vpu_fpga_mem.o ftv310_vpu_power.o ftv310_vpu_debugfs.o ftv310_vpu_voltage.o ftv310_vpu_trace.o
#FTV310_VPU_CFLAGS=-DUSE_CMA

ifeq ($(PHY_FPGA_MEM),y)
	ftv310_vpu_driver-objs += ftv310_vpu_fpga_mem.o
	FTV310_VPU_CFLAGS += -DPHY_FPGA_MEM -DPHY_FPGA_PCIE
	PCIE=y
endif

ifeq ($(PCIE),y)
	ftv310_vpu_driver-objs += ftv310_vpu_pcie.o
	FTV310_VPU_CFLAGS += -DPCIE_EN
endif

ifeq ($(PHY_VCE),y)
	ftv310_vpu_driver-objs += ftv310e.o
	FTV310_VPU_CFLAGS += -DHAS_VCE
endif

ifeq ($(PHY_VCD),y)
	ftv310_vpu_driver-objs += ftv310_vpu_dec.o
	FTV310_VPU_CFLAGS += -DHAS_VCD
endif

ifeq ($(PHY_VCMD),y)
	ftv310_vpu_driver-objs += ftv310_vpu_vcmd.o ftv310_vpu_vcmd_slice.o
	FTV310_VPU_CFLAGS += -DHAS_VCMD
endif

ifeq ($(PHY_CACHE),y)
	ftv310_vpu_driver-objs += ftv310_vpu_cache.o
	FTV310_VPU_CFLAGS += -DHAS_CACHECORE
endif

ifeq ($(PHY_DEC400),y)
	ftv310_vpu_driver-objs += ftv310_vpu_dec400.o
	FTV310_VPU_CFLAGS += -DHAS_DEC400
endif

ifeq ($(PHY_AXIFE),y)
	ftv310_vpu_driver-objs += ftv310_vpu_axife.o
	FTV310_VPU_CFLAGS += -DHAS_AXIFE
endif

ifeq ($(PHY_MMU),y)
	ftv310_vpu_driver-objs += ftv310_vpu_mmu.o
	FTV310_VPU_CFLAGS += -DHAS_MMU
endif

ifeq ($(PHY_SUPPORT_PM),y)
	FTV310_VPU_CFLAGS += -DPHY_CONFIG_PM
endif

SIMICS=n
ifeq ($(SIMICS),y)
	FTV310_VPU_CFLAGS += -DSIMICS_TEST
endif

FTV310_VPU_CFLAGS += -I$(src)
ccflags-y += $(FTV310_VPU_CFLAGS)


KDIR ?= /lib/modules/$(shell uname -r)/build

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean::
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	-rm -f .version
