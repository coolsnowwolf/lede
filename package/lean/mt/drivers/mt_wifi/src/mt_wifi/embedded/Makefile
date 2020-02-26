ifeq ($(WIFI_MODE),)
RT28xx_MODE = AP
else
RT28xx_MODE = $(WIFI_MODE)
endif

ifeq ($(TARGET),)
TARGET = LINUX
endif

# CHIPSET
ifeq ($(CHIPSET),)
CHIPSET = mt7622 mt7615
endif

MODULE = $(word 1, $(CHIPSET))

#OS ABL - YES or NO
OSABL = NO

ifneq ($(TARGET),THREADX)
#RT28xx_DIR = home directory of RT28xx source code
RT28xx_EMBEDDED_DIR = $(shell pwd)
RT28xx_DIR = $(shell cd ..;pwd)
#RT28xx_BIN_DIR = parent directory of bin folder, used by bin2h
RT28xx_BIN_DIR = $(shell cd ../..;pwd)
RT28xx_OS_DIR = $(RT28xx_DIR)/os
endif

include $(RT28xx_OS_DIR)/linux/config.mk

RTMP_SRC_DIR = $(RT28xx_DIR)/RT$(MODULE)

ifeq ($(PLATFORM),)
#PLATFORM: Target platform
PLATFORM = PC
#PLATFORM = BB_SOC
#PLATFORM = IKANOS_V160
#PLATFORM = IKANOS_V180
#PLATFORM = SIGMA
#PLATFORM = SIGMA_8622
#PLATFORM = INIC
#PLATFORM = STAR
#PLATFORM = IXP
#PLATFORM = INF_TWINPASS
#PLATFORM = INF_DANUBE
#PLATFORM = INF_AR9
#PLATFORM = INF_VR9
#PLATFORM = BRCM_6358
#PLATFORM = INF_AMAZON_SE
#PLATFORM = CAVM_OCTEON
#PLATFORM = CMPC
#PLATFORM = SMDK
#PLATFORM = RMI
#PLATFORM = RMI_64
#PLATFORM = KODAK_DC
#PLATFORM = DM6446
#PLATFORM = FREESCALE8377
#PLATFORM = BL2348
#PLATFORM = BL23570
#PLATFORM = BLUBB
#PLATFORM = BLPMP
#PLATFORM = MT85XX
#PLATFORM = MT53XX
#PLATFORM = NXP_TV550
#PLATFORM = MVL5
#PLATFORM = UBICOM_IPX8
#PLATFORM = INTELP6
#PLATFORM = MT7620
#PLATFORM = MT7621
#PLATFORM = MT7622
endif

#APSOC
ifeq ($(CHIPSET),mt7628)
PLATFORM = MT7628
endif

#RELEASE Package
RELEASE = FormalRelease

ifeq ($(TARGET),LINUX)
MAKE = make
endif

ifeq ($(TARGET), UCOS)
MAKE = make
endif
ifeq ($(TARGET),THREADX)
MAKE = gmake
endif

ifeq ($(TARGET), ECOS)
MAKE = make
MODULE = $(shell pwd | sed "s/.*\///" ).o
export MODULE
endif

ifeq ($(PLATFORM),BB_SOC)
LINUX_SRC = $(KERNEL_DIR)
#CROSS_COMPILE = /opt/trendchip/mips-linux-uclibc/usr/bin/mips-linux-uclibc-
endif

ifeq ($(PLATFORM),UBICOM_IPX8)
LINUX_SRC = /home/sample/Customers/UBICOM/ubicom-linux-dist-2.1.1/linux-2.6.x
CROSS_COMPILE = ubicom32-elf-
endif

ifeq ($(PLATFORM),IKANOS_V160)
LINUX_SRC = /home/sample/projects/LX_2618_RG_5_3_00r4_SRC/linux-2.6.18
CROSS_COMPILE = mips-linux-
endif

ifeq ($(PLATFORM),IKANOS_V180)
LINUX_SRC = /home/sample/projects/LX_BSP_VX180_5_4_0r1_ALPHA_26DEC07/linux-2.6.18
CROSS_COMPILE = mips-linux-
endif

ifeq ($(PLATFORM),SIGMA)
LINUX_SRC = /root/sigma/smp86xx_kernel_source_2.7.172.0/linux-2.6.15
CROSS_COMPILE = /root/sigma/smp86xx_toolchain_2.7.172.0/build_mipsel_nofpu/staging_dir/bin/mipsel-linux-
endif

ifeq ($(PLATFORM),SIGMA_8622)
LINUX_SRC = /home/snowpin/armutils_2.5.120.1/build_arm/linux-2.4.22-em86xx
CROSS_COMPILE = /home/snowpin/armutils_2.5.120.1/toolchain/bin/arm-elf-
CROSS_COMPILE_INCLUDE = /home/snowpin/armutils_2.5.120.1/toolchain/lib/gcc-lib/arm-elf/2.95.3
endif

ifeq ($(PLATFORM),INIC)
UCOS_SRC = /opt/uCOS/iNIC_rt2880
CROSS_COMPILE = /usr/bin/mipsel-linux-
endif

ifeq ($(PLATFORM),STAR)
LINUX_SRC = /opt/star/kernel/linux-2.4.27-star
CROSS_COMPILE = /opt/star/tools/arm-linux/bin/arm-linux-
endif

ifeq ($(PLATFORM),RMI)
LINUX_SRC = /opt/rmi/1.7.0/linux/src/
CROSS_COMPILE = /opt/rmi/1.7.0/mipscross/nptl/bin/mips64-unknown-linux-gnu-
endif

ifeq ($(PLATFORM),RMI_64)
LINUX_SRC = /opt/rmi/1.7.0/linux_64/src/
CROSS_COMPILE = /opt/rmi/1.7.0/mipscross/nptl/bin/mips64-unknown-linux-gnu-
endif

ifeq ($(PLATFORM),FREESCALE8377)
LINUX_SRC = /opt/ltib-mpc8377_rds-20090309/rpm/BUILD/linux-2.6.25 
CROSS_COMPILE = /opt/freescale/usr/local/gcc-4.2.187-eglibc-2.5.187/powerpc-linux-gnu/bin/powerpc-linux-gnu-
endif

ifeq ($(PLATFORM),BL2348)
LINUX_SRC = /home/sample/Customers/BroadLight/bl234x-linux-2.6.21-small-v29
CROSS_COMPILE = mips-wrs-linux-gnu-
endif

ifeq ($(PLATFORM),BL23570)
LINUX_SRC = /home/FIBERHOME/linux-2.6.34.8
CROSS_COMPILE = mips-wrs-linux-gnu-mips_74k_softfp-glibc_small-
ARCH:=mips
export $ARCH
endif


ifeq ($(PLATFORM),BLUBB)
LINUX_SRC = /home/sample/Customers/BroadLight/UBB/gmp20/linux-2.6.21-small
CROSS_COMPILE = mips-wrs-linux-gnu-
endif

ifeq ($(PLATFORM),BLPMP)
LINUX_SRC = /home/sample/Customers/BroadLight/UBB/pmp16/bl234x-linux-2.6.21-small-v30.2
CROSS_COMPILE = mips-wrs-linux-gnu-
endif

ifeq ($(PLATFORM),PC)
# Linux 2.6
LINUX_SRC = /lib/modules/$(shell uname -r)/build
# Linux 2.4 Change to your local setting
#LINUX_SRC = /usr/src/linux-2.4
LINUX_SRC_MODULE = /lib/modules/$(shell uname -r)/kernel/drivers/net/wireless/
CROSS_COMPILE = 
endif

ifeq ($(PLATFORM),INTELP6)
LINUX_SRC = /tftpboot/IntelCE-20.0.11052.243193/project_build_i686/IntelCE/kernel-20.0.11024.238456/linux-2.6.35
CROSS_COMPILE = /tftpboot/IntelCE-20.0.11052.243193/build_i686/i686-linux-elf/bin/i686-cm-linux-
endif

ifeq ($(PLATFORM),IXP)
LINUX_SRC = /project/stable/Gmtek/snapgear-uclibc/linux-2.6.x
CROSS_COMPILE = arm-linux-
endif

ifeq ($(PLATFORM),INF_TWINPASS)
# Linux 2.6
#LINUX_SRC = /lib/modules/$(shell uname -r)/build
# Linux 2.4 Change to your local setting
LINUX_SRC = /project/stable/twinpass/release/2.0.1/source/kernel/opensource/linux-2.4.31/
CROSS_COMPILE = mips-linux-
endif

ifeq ($(PLATFORM),INF_DANUBE)
LINUX_SRC = /opt/danube/sdk/linux-2.6.16.x
CROSS_COMPILE = mips-linux-
ROOTDIR = /opt/danube/sdk
export ROOTDIR
endif

ifeq ($(PLATFORM),INF_AR9)
LINUX_SRC = /root/ar9/xR9_BSP1.2.2.0/source/kernel/opensource/linux-2.6.20/
CROSS_COMPILE = /root/ar9/ifx-lxdb26-1.0.2/gcc-3.4.4/toolchain-mips/bin/
endif

ifeq ($(PLATFORM),INF_VR9)
LINUX_SRC = /home/public/lantiq/VR9/UGW-4.2/build_dir/linux-ifxcpe_platform_vr9/linux-2.6.20.19
CROSS_COMPILE = /home/public/lantiq/VR9/UGW-4.2/staging_dir/toolchain-mips_gcc-3.4.6_uClibc-0.9.29/bin/mips-linux-
endif

ifeq ($(PLATFORM),BRCM_6358)
LINUX_SRC = 
CROSS_COMPILE = 
endif

ifeq ($(PLATFORM),INF_AMAZON_SE)
# Linux 2.6
#LINUX_SRC = /lib/modules/$(shell uname -r)/build
# Linux 2.4 Change to your local setting
LINUX_SRC = /backup/ifx/3.6.2.2/source/kernel/opensource/linux-2.4.31
#CROSS_COMPILE = mips-linux-
#LINUX_SRC = /project/Infineon/3.6.2.2/source/kernel/opensource/linux-2.4.31
CROSS_COMPILE = /opt/uclibc-toolchain/ifx-lxdb-1-2-3-external/gcc-3.3.6/toolchain-mips/R0208V35/mips-linux-uclibc/bin/
endif

ifeq ($(PLATFORM),ST)
LINUX_SRC = /opt/STM/STLinux-2.2/devkit/sources/kernel/linux0039
CROSS_COMPILE = /opt/STM/STLinux-2.2/devkit/sh4/bin/sh4-linux-
ARCH := sh
export ARCH
endif

ifeq ($(PLATFORM),CAVM_OCTEON)
OCTEON_ROOT = /usr/local/Cavium_Networks/OCTEON-SDK
LINUX_SRC = $(OCTEON_ROOT)/linux/kernel_2.6/linux
CROSS_COMPILE = mips64-octeon-linux-gnu-
endif

ifeq ($(PLATFORM),CMPC)
LINUX_SRC = /opt/fvt_11N_SDK_0807/fvt131x_SDK_11n/linux-2.6.17
CROSS_COMPILE =
endif

ifeq ($(PLATFORM),SMDK)
LINUX_SRC = /home/bhushan/itcenter/may28/linux-2.6-samsung
CROSS_COMPILE = /usr/local/arm/4.2.2-eabi/usr/bin/arm-linux-
endif

ifeq ($(PLATFORM),KODAK_DC)
SKD_SRC = C:/SigmaTel/DC1250_SDK_v1-9/sdk
CROSS_COMPILE = $(cc)
endif

ifeq ($(PLATFORM),DM6446)
LINUX_SRC = /home/fonchi/work/soc/ti-davinci
endif

ifeq ($(PLATFORM),MT85XX)
LINUX_SRC = /home/john/MTK/BDP_Linux/linux-2.6.27
CROSS_COMPILE = armv6z-mediatek-linux-gnueabi-
endif

ifeq ($(PLATFORM),MT53XX)
TARGET = LINUX
#****** For system auto build ******
#LINUX_SRC=$(KERNEL_OBJ_ROOT)/$(KERNEL_VER)/$(KERNEL_CONFIG)_modules
#****** For local build ******
# uncomment the following lines
VM_LINUX_ROOT ?= $(word 1, $(subst /vm_linux/,/vm_linux /, $(shell pwd)))
LINUX_ROOT ?= $(VM_LINUX_ROOT)
LINUX_SRC=$(VM_LINUX_ROOT)/output/Sony_android/Fiji_EU_JB/rel/obj/kernel/chiling/kernel/linux-3.4/_android_smp_mod_defconfig_modules
OBJ_ROOT ?= $(VM_LINUX_ROOT)/output/Sony_android/Fiji_EU_JB/rel/obj
export KERNEL_OBJ_ROOT=$(LINUX_SRC)/../..
ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE=/mtkoss/gnuarm/vfp_4.5.1_2.6.27_cortex-a9-rhel4/i686/bin/armv7a-mediatek451_001_vfp-linux-gnueabi-
endif
ifeq "$(CC)" "gcc"
CC ?= $(CROSS_COMPILE)gcc
endif
$(warning =============================================)
$(warning CC=$(CC) for wifi driver LINUX_SRC=$(LINUX_SRC))
$(warning TARGET=$(TARGET))
$(warning =============================================)
endif

ifeq ($(PLATFORM),NXP_TV550) 
LINUX_SRC = /data/tv550/kernel/linux-2.6.28.9
LINUX_SRC_MODULE = /data/tv550/kernel/linux-2.6.28.9/drivers/net/wireless
CROSS_COMPILE = /opt/embeddedalley/nxp_tv550/bin/mipsel-linux-
endif

ifeq ($(PLATFORM),MVL5)
LINUX_SRC = /home2/charlestu/AP-VT3426/linux-2.6.18
CROSS_COMPILE = /opt/montavista/pro/devkit/arm/v5t_le_mvl5/bin/arm_v5t_le-
endif

ifeq ($(PLATFORM),MT7620)
LINUX_SRC = /home/share/src/MT7601/AP/RT288x_SDK/source/linux-2.6.36.x
CROSS_COMPILE = /opt/buildroot-gcc342/bin/mipsel-linux-
endif

ifeq ($(PLATFORM),MT7621)
LINUX_SRC = /mtkoss/linux_headers/linux-headers-3.10.14-mips
CROSS_COMPILE = /opt/buildroot-gcc463/usr/bin/mipsel-linux-
ARCH = mips
export $ARCH
endif

ifeq ($(PLATFORM),MT7628)
LINUX_SRC = /root/Working/MT7628_SDK_2013_0912/RT288x_SDK/source/linux-2.6.36.x
CROSS_COMPILE = /opt/buildroot-gcc342/bin/mipsel-linux-
endif

ifeq ($(PLATFORM),MT7622)
LINUX_SRC = /mtkoss/linux_headers/linux-headers-4.4.24-arm64/
CROSS_COMPILE = /opt/buildroot-gcc492_arm64/usr/bin/aarch64-linux-
ARCH = arm64
export $ARCH
endif

export OSABL RT28xx_DIR RT28xx_BIN_DIR RT28xx_OS_DIR RT28xx_EMBEDDED_DIR RT28xx_MODE LINUX_SRC CROSS_COMPILE CROSS_COMPILE_INCLUDE PLATFORM RELEASE CHIPSET MODULE RTMP_SRC_DIR LINUX_SRC_MODULE TARGET HAS_WOW_SUPPORT HAS_FPGA_MODE HAS_RX_CUT_THROUGH

# The targets that may be used.
PHONY += all build_tools test UCOS THREADX LINUX release prerelease clean uninstall install libwapi osabl sdk_build_tools

ifeq ($(TARGET),LINUX)
all: build_tools $(TARGET) plug_in
else
all: $(TARGET)
endif 

build_sku_tables:
	$(MAKE) -C $(RT28xx_DIR)/txpwr
	$(RT28xx_DIR)/txpwr/dat2h

build_tools:
	$(MAKE) -C tools
	$(RT28xx_EMBEDDED_DIR)/tools/bin2h

sdk_build_tools:
	if [ -f $(RT28xx_DIR)/eeprom_log ]; then \
		rm -f $(RT28xx_DIR)/eeprom_log
	fi
	echo $(EE_TYPE) >> eeprom_log
	echo $(CHIPSET) >> eeprom_log
	if [ -f $(RT28xx_DIR)/eeprom/SA/MT7603E_EEPROM.bin ]; then \
		echo 'find SA/MT7603E_EEPROM.bin' >> eeprom_log ; \
		cp -f $(RT28xx_DIR)/eeprom/SA/MT7603E_EEPROM.bin $(RT28xx_DIR)/eeprom/MT7603E_EEPROM.bin ; \
	else \
		cp -f $(RT28xx_DIR)/eeprom/$(EE_TYPE)/MT7603E_EEPROM.bin $(RT28xx_DIR)/eeprom/MT7603E_EEPROM.bin ; \
	fi
	$(MAKE) -C tools
	$(RT28xx_EMBEDDED_DIR)/tools/bin2h
#	rm -f $(RT28xx_DIR)/eeprom/SA/MT7603E_EEPROM.bin

test:
	$(MAKE) -C tools test

UCOS:
	$(MAKE) -C os/ucos/ MODE=$(RT28xx_MODE)
	echo $(RT28xx_MODE)

ECOS:
	$(MAKE) -C os/ecos/ MODE=$(RT28xx_MODE)
	cp -f os/ecos/$(MODULE) $(MODULE)

THREADX:
	$(MAKE) -C $(RT28xx_OS_DIR)/Threadx -f $(RT28xx_OS_DIR)/ThreadX/Makefile

LINUX:
ifneq (,$(findstring 2.4,$(LINUX_SRC)))

ifeq ($(OSABL),YES)
	cp -f $(RT28xx_OS_DIR)/linux/Makefile.4.util $(RT28xx_OS_DIR)/linux/Makefile
	$(MAKE) -C $(RT28xx_OS_DIR)/linux/
endif

	cp -f $(RT28xx_OS_DIR)/linux/Makefile.4 $(RT28xx_OS_DIR)/linux/Makefile
	$(MAKE) -C $(RT28xx_OS_DIR)/linux/

ifeq ($(OSABL),YES)
	cp -f $(RT28xx_OS_DIR)/linux/Makefile.4.netif $(RT28xx_OS_DIR)/linux/Makefile
	$(MAKE) -C $(RT28xx_OS_DIR)/linux/
endif

else

ifeq ($(OSABL),YES)
	cp -f $(RT28xx_OS_DIR)/linux/Makefile.6.util $(RT28xx_OS_DIR)/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_OS_DIR)/linux modules
endif

	cp -f $(RT28xx_OS_DIR)/linux/Makefile.6 $(RT28xx_OS_DIR)/linux/Makefile
ifeq ($(PLATFORM),DM6446)
	$(MAKE)  ARCH=arm CROSS_COMPILE=arm_v5t_le- -C  $(LINUX_SRC) SUBDIRS=$(RT28xx_OS_DIR)/linux modules
else ifeq ($(PLATFORM),FREESCALE8377)
	$(MAKE) ARCH=powerpc CROSS_COMPILE=$(CROSS_COMPILE) -C  $(LINUX_SRC) SUBDIRS=$(RT28xx_OS_DIR)/linux modules
else ifeq ($(PLATFORM), $(filter $(PLATFORM), MT7622 MT7621))
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_OS_DIR)/linux modules
else
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_OS_DIR)/linux modules
endif

ifeq ($(OSABL),YES)
	cp -f $(RT28xx_OS_DIR)/linux/Makefile.6.netif $(RT28xx_OS_DIR)/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_OS_DIR)/linux modules
endif

endif

check_build:
	$(RT28xx_EMBEDDED_DIR)/tools/check_build_script.sh

plug_in:
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_EMBEDDED_DIR)/tools/plug_in MODULE_FLAGS="$(WFLAGS)"

release: build_tools
	$(MAKE) -C $(RT28xx_EMBEDDED_DIR)/striptool -f Makefile.release clean
	$(MAKE) -C $(RT28xx_EMBEDDED_DIR)/striptool -f Makefile.release
	striptool/striptool.out
ifeq ($(RELEASE), DPO)
	gcc -o embedded/striptool/banner striptool/banner.c
	./embedded/striptool/banner -b embedded/striptool/copyright.gpl -s DPO/ -d DPO_GPL -R
	./embedded/striptool/banner -b embedded/striptool/copyright.frm -s DPO_GPL/include/firmware.h
endif

prerelease:
ifeq ($(MODULE), 2880)
	$(MAKE) -C $(RT28xx_OS_DIR)/linux -f Makefile.release.2880 prerelease
else
	$(MAKE) -C $(RT28xx_OS_DIR)/linux -f Makefile.release prerelease
endif
	cp $(RT28xx_OS_DIR)/linux/Makefile.DPB $(RTMP_SRC_DIR)/os/linux/.
	cp $(RT28xx_OS_DIR)/linux/Makefile.DPA $(RTMP_SRC_DIR)/os/linux/.
	cp $(RT28xx_OS_DIR)/linux/Makefile.DPC $(RTMP_SRC_DIR)/os/linux/.
ifeq ($(RT28xx_MODE),STA)
	cp $(RT28xx_OS_DIR)/linux/Makefile.DPD $(RTMP_SRC_DIR)/os/linux/.
	cp $(RT28xx_OS_DIR)/linux/Makefile.DPO $(RTMP_SRC_DIR)/os/linux/.
endif	

clean:
ifeq ($(TARGET), LINUX)
	cp -f $(RT28xx_OS_DIR)/linux/Makefile.clean $(RT28xx_OS_DIR)/linux/Makefile
	$(MAKE) -C $(RT28xx_OS_DIR)/linux clean
	rm -rf $(RT28xx_OS_DIR)/linux/Makefile
endif	
ifeq ($(TARGET), UCOS)
	$(MAKE) -C $(RT28xx_OS_DIR)/ucos clean MODE=$(RT28xx_MODE)
endif
ifeq ($(TARGET), ECOS)
	$(MAKE) -C $(RT28xx_OS_DIR)/ecos clean MODE=$(RT28xx_MODE)
endif

uninstall:
ifeq ($(TARGET), LINUX)
ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	$(MAKE) -C $(RT28xx_OS_DIR)/linux -f Makefile.4 uninstall
else
	$(MAKE) -C $(RT28xx_OS_DIR)/linux -f Makefile.6 uninstall
endif
endif

install:
ifeq ($(TARGET), LINUX)
ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	$(MAKE) -C $(RT28xx_OS_DIR)/linux -f Makefile.4 install
else
	$(MAKE) -C $(RT28xx_OS_DIR)/linux -f Makefile.6 install
endif
endif

libwapi:
ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	cp -f embedded/os/linux/Makefile.libwapi.4 $(RT28xx_OS_DIR)/linux/Makefile
	$(MAKE) -C $(RT28xx_OS_DIR)/linux/
else
	cp -f embedded/os/linux/Makefile.libwapi.6 $(RT28xx_OS_DIR)/linux/Makefile	
	$(MAKE) -C  $(LINUX_SRC) SUBDIRS=$(RT28xx_OS_DIR)/linux modules	
endif	

osutil:
ifeq ($(OSABL),YES)
ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	cp -f os/linux/Makefile.4.util $(RT28xx_OS_DIR)/linux/Makefile
	$(MAKE) -C $(RT28xx_DIR)/os/linux/
else
	cp -f os/linux/Makefile.6.util $(RT28xx_OS_DIR)/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
endif
endif

osnet:
ifeq ($(OSABL),YES)
ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	cp -f os/linux/Makefile.4.netif $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(RT28xx_DIR)/os/linux/
else
	cp -f os/linux/Makefile.6.netif $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
endif
endif

osdrv:
ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	cp -f os/linux/Makefile.4 $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(RT28xx_DIR)/os/linux/
else
	cp -f os/linux/Makefile.6 $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
endif

# Declare the contents of the .PHONY variable as phony.  We keep that information in a variable
.PHONY: $(PHONY)



