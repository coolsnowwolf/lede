RT28xx_MODE = AP



TARGET = LINUX


# CHIPSET
# rt2860, rt2870, rt2880, rt2070, rt3070, rt3090, rt3572, rt3062, rt3562, rt3593, rt3573
# rt3562(for rt3592), rt3050, rt3350, rt3352, rt5350, rt5370, rt5390, rt5572, rt5592, 
# rt8592(for rt85592), 
# mt7601e, mt7601u,
# mt7650e, mt7630e, mt7610e, mt7650u, mt7630u, mt7610u
# mt7662e, mt7632e, mt7612e, mt7662u, mt7632u, mt7612u
# mt7603e, mt7603u, mt7636u, mt7636s


CHIPSET = mt7603e


MODULE = $(word 1, $(CHIPSET))

#OS ABL - YES or NO
OSABL = NO

#Build Prealloc ko
PREALLOC = NO

ifneq ($(TARGET),THREADX)
#RT28xx_DIR = home directory of RT28xx source code
RT28xx_DIR = $(shell pwd)
endif

include $(RT28xx_DIR)/os/linux/config.mk

RTMP_SRC_DIR = $(RT28xx_DIR)/RT$(MODULE)

#PLATFORM: Target platform
PLATFORM = PC
#PLATFORM = MT53XX
#PLATFORM = PC_AUTO_BUILD
#PLATFORM = BB_SOC
#PLATFORM = 5VT
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
#PLATFORM = RALINK_2880
#PLATFORM = RALINK_3052
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
#PLATFORM = NXP_TV550
#PLATFORM = MVL5
#PLATFORM = RALINK_3352
#PLATFORM = UBICOM_IPX8
#PLATFORM = INTELP6
#PLATFORM = MT7620
#PLATFORM = MT7621
#PLATFORM = MSTAR

#APSOC
ifeq ($(MODULE),3050)
PLATFORM = RALINK_3050
endif
ifeq ($(MODULE),3052)
PLATFORM = RALINK_3052
endif
ifeq ($(MODULE),3350)
PLATFORM = RALINK_3050
endif
ifeq ($(MODULE),3352)
PLATFORM = RALINK_3352
endif
ifeq ($(CHIPSET),mt7628)
PLATFORM = MT7628
endif

#RELEASE Package
RELEASE = DPA


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

ifeq ($(PLATFORM),5VT)
LINUX_SRC = /home/ralink-2860-sdk-5vt-distribution/linux-2.6.17
CROSS_COMPILE = /opt/crosstool/uClibc_v5te_le_gcc_4_1_1/bin/arm-linux-
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

ifeq ($(PLATFORM), RALINK_2880)
LINUX_SRC = /project/stable/RT288x/RT288x_SDK/source/linux-2.4.x
CROSS_COMPILE = /opt/buildroot-gdb/bin/mipsel-linux-
endif

ifeq ($(PLATFORM),RALINK_3052)
LINUX_SRC = /home/peter/ap_soc/SDK_3_3_0_0/RT288x_SDK/source/linux-2.6.21.x
CROSS_COMPILE = /opt/buildroot-gcc342/bin/mipsel-linux-uclibc-
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

ifeq ($(PLATFORM),RALINK_3352)
LINUX_SRC = /home/sample/3352/RT288x_SDK/source/linux-2.6.21.x
CROSS_COMPILE = /opt/buildroot-gcc342/bin/mipsel-linux-
endif

ifeq ($(PLATFORM),KODAK_DC)
SKD_SRC = C:/SigmaTel/DC1250_SDK_v1-9/sdk
CROSS_COMPILE = $(cc)
endif

ifeq ($(PLATFORM),DM6446)
LINUX_SRC = /home/fonchi/work/soc/ti-davinci
endif

ifeq ($(PLATFORM),MT85XX)
ifeq ($(OSABL),YES)
LINUX_SRC = $(RT28xx_DIR)/../../../../../build_linux
else
LINUX_SRC = $(RT28xx_DIR)/../../../../build_linux
endif
ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE=armv7a-mediatek451_001_vfp-linux-gnueabi-
endif
CC=$(CROSS_COMPILE)gcc
$(warning =============================================)
$(warning CC=$(CC) for wifi driver)
$(warning =============================================)
endif

ifeq ($(PLATFORM),MT53XX)
TARGET = LINUX
#****** For system auto build ******
#LINUX_SRC=$(KERNEL_OBJ_ROOT)/$(KERNEL_VER)/$(KERNEL_CONFIG)_modules
#****** For local build ******
# uncomment the following lines
VM_LINUX_ROOT ?= $(word 1, $(subst /vm_linux/,/vm_linux /, $(shell pwd)))
LINUX_ROOT ?= $(VM_LINUX_ROOT)
LINUX_SRC=$(VM_LINUX_ROOT)/output/mtk_android/mt5399_cn_android/rel/obj/kernel/chiling/kernel/linux-3.10/mt5399_android_smp_mod_defconfig_modules
OBJ_ROOT ?= $(VM_LINUX_ROOT)/output/mtk_android/mt5399_cn_android/rel/obj
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

ifeq ($(PLATFORM),PC_AUTO_BUILD)
LINUX_SRC=/proj/srv_wcnautobuild/external/linux-3.10.59/
#CROSS_COMPILE=/proj/srv_wcnautobuild/external/gcc-4.9.2/bin/
CROSS_COMPILE=
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
LINUX_SRC = /root/SDK_4_2_0_0/RT288x_SDK/source/linux-2.6.36.x 
CROSS_COMPILE = /opt/buildroot-gcc463/usr/bin/mipsel-linux-
endif


ifeq ($(PLATFORM),MT7628)
LINUX_SRC = /root/Working/MT7628_SDK_2013_0912/RT288x_SDK/source/linux-2.6.36.x
CROSS_COMPILE = /opt/buildroot-gcc342/bin/mipsel-linux-
endif


ifeq ($(PLATFORM),MSTAR)
PREALLOC = YES

#628 TV Platform
#LINUX_SRC = /proj/mtk05650/share/mstar/628/kernel/RedLion/3.1.10/
#LINUX_SRC = /proj/mtk05650/share/mstar/628/kernel/3.10.23/3.10.23/
#LINUX_SRC = /proj/mtk05650/share/mstar/628/kernel/3.1.10_Madison_TVOS/3.1.10/

#918 TV Android4.3 Platform
#LINUX_SRC = /proj/mtk05650/share/mstar/918_TV/android4.3/kernel/3.1.10_Napoli_tvos/3.1.10/

#918 Box Android4.4 Platform
#LINUX_SRC = /proj/mtk05650/share/mstar/toolchain/918_Box_android4.4/3.1.10_Napoli_tvos_0616_new_kernel/3.1.10/

#928 Box Android4.4 Platform
#LINUX_SRC = /proj/mtk05650/share/mstar/928/928_kernel_0716/3.10.23/
#LINUX_SRC = /proj/mtk05650/share/mstar/928/928_kernel_0722/3.10.23/
LINUX_SRC = /proj/mtk05650/share/mstar/928/20140710_KERN-3.10.23-00911089/3.10.23/

#ToolChian Setting
#CROSS_COMPILE = /proj/mtk05650/share/mstar/628/toolchain/arm-2012.09/bin/arm-none-linux-gnueabi-
CROSS_COMPILE = /proj/mtk05650/share/mstar/toolchain/arm-2010.09/bin/arm-none-linux-gnueabi-
#CROSS_COMPILE = /proj/mtk05650/share/mstar/toolchain/918_Box_android4.4/arm-2012.09/bin/arm-none-linux-gnueabi-
#CROSS_COMPILE = /proj/mtk05650/share/mstar/toolchain/918_Box_android4.4/arm-2012.09/bin/arm-none-linux-gnueabi-
endif

export OSABL RT28xx_DIR RT28xx_MODE LINUX_SRC CROSS_COMPILE CROSS_COMPILE_INCLUDE PLATFORM RELEASE CHIPSET MODULE RTMP_SRC_DIR LINUX_SRC_MODULE TARGET HAS_WOW_SUPPORT

# The targets that may be used.
PHONY += all build_tools test UCOS THREADX LINUX release prerelease clean uninstall install libwapi osabl sdk_build_tools

ifeq ($(TARGET),LINUX)
all: build_tools $(TARGET) plug_in
else
all: $(TARGET)
endif 

build_tools:
	$(MAKE) -C tools
	$(RT28xx_DIR)/tools/bin2h

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
	$(RT28xx_DIR)/tools/bin2h
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
	$(MAKE) -C $(RT28xx_DIR)/os/Threadx -f $(RT28xx_DIR)/os/ThreadX/Makefile

LINUX:
ifneq (,$(findstring 2.4,$(LINUX_SRC)))

ifeq ($(OSABL),YES)
	cp -f os/linux/Makefile.4.util $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(RT28xx_DIR)/os/linux/
endif

	cp -f os/linux/Makefile.4 $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(RT28xx_DIR)/os/linux/

ifeq ($(OSABL),YES)
	cp -f os/linux/Makefile.4.netif $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(RT28xx_DIR)/os/linux/
endif

ifeq ($(RT28xx_MODE),AP)
ifneq ($(PLATFORM),BB_SOC)
	cp -f $(RT28xx_DIR)/os/linux/$(MODULE)_ap.o /tftpboot
ifeq ($(OSABL),YES)
	cp -f $(RT28xx_DIR)/os/linux/rtutil$(MODULE)_ap.o /tftpboot
	cp -f $(RT28xx_DIR)/os/linux/rtnet$(MODULE)_ap.o /tftpboot
endif
ifeq ($(PLATFORM),INF_AMAZON_SE)
	cp -f /tftpboot/rt2870ap.o /backup/ifx/build/root_filesystem/lib/modules/2.4.31-Amazon_SE-3.6.2.2-R0416_Ralink/kernel/drivers/net
endif
endif
else	
ifeq ($(RT28xx_MODE),APSTA)
	cp -f $(RT28xx_DIR)/os/linux/$(MODULE)_apsta.o /tftpboot
ifeq ($(OSABL),YES)
	cp -f $(RT28xx_DIR)/os/linux/rtutil$(MODULE)_apsta.o /tftpboot
	cp -f $(RT28xx_DIR)/os/linux/rtnet$(MODULE)_apsta.o /tftpboot
endif
else
	cp -f $(RT28xx_DIR)/os/linux/$(MODULE)_sta.o /tftpboot
ifeq ($(OSABL),YES)
	cp -f $(RT28xx_DIR)/os/linux/rtutil$(MODULE)_sta.o /tftpboot
	cp -f $(RT28xx_DIR)/os/linux/rtnet$(MODULE)_sta.o /tftpboot
endif
endif
endif	
else

ifeq ($(OSABL),YES)
	cp -f os/linux/Makefile.6.util $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
endif
ifeq ($(PREALLOC), YES)
	cp -f PREALLOC/os/linux/Makefile.6.prealloc PREALLOC/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/PREALLOC/os/linux modules
	$(SHELL) cp_prealloc.sh
endif
	cp -f os/linux/Makefile.6 $(RT28xx_DIR)/os/linux/Makefile
ifeq ($(PLATFORM),DM6446)
	$(MAKE)  ARCH=arm CROSS_COMPILE=arm_v5t_le- -C  $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
else
ifeq ($(PLATFORM),FREESCALE8377)
	$(MAKE) ARCH=powerpc CROSS_COMPILE=$(CROSS_COMPILE) -C  $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
else
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
endif
endif

ifeq ($(OSABL),YES)
	cp -f os/linux/Makefile.6.netif $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
endif

ifeq ($(RT28xx_MODE),AP)
ifneq ($(PLATFORM),BB_SOC)
	cp -f $(RT28xx_DIR)/os/linux/$(MODULE)_ap.ko /tftpboot
ifeq ($(OSABL),YES)
	cp -f $(RT28xx_DIR)/os/linux/$(MODULE)_ap_util.ko /tftpboot
	cp -f $(RT28xx_DIR)/os/linux/$(MODULE)_ap_net.ko /tftpboot
endif
else
	rm -f os/linux/$(MODULE)_ap.ko.lzma
	/root/bin/lzma e os/linux/$(MODULE)_ap.ko os/linux/$(MODULE)_ap.ko.lzma
endif
else	
ifeq ($(RT28xx_MODE),APSTA)
	cp -f $(RT28xx_DIR)/os/linux/$(MODULE)_apsta.ko /tftpboot
ifeq ($(OSABL),YES)
	cp -f $(RT28xx_DIR)/os/linux/$(MODULE)_apsta_util.ko /tftpboot
	cp -f $(RT28xx_DIR)/os/linux/$(MODULE)_apsta_net.ko /tftpboot
endif
else
ifeq ($(PLATFORM),PC_AUTO_BUILD)
else
	cp -f $(RT28xx_DIR)/os/linux/$(MODULE)_sta.ko /tftpboot
endif
ifeq ($(OSABL),YES)
	cp -f $(RT28xx_DIR)/os/linux/$(MODULE)_sta_util.ko /tftpboot
	cp -f $(RT28xx_DIR)/os/linux/$(MODULE)_sta_net.ko /tftpboot
endif
endif
endif
endif
ifeq ($(PLATFORM),MT85XX)
	mkdir -p $(RT28xx_DIR)/../../../../../BDP_Generic/build_linux_ko/src/driver/wlan/
	cp -f $(RT28xx_DIR)/os/linux/mt7603u_sta.ko $(RT28xx_DIR)/../../../../../BDP_Generic/build_linux_ko/src/driver/wlan/
endif

plug_in:
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/tools/plug_in MODULE_FLAGS="$(WFLAGS)"

release: build_tools
	$(MAKE) -C $(RT28xx_DIR)/striptool -f Makefile.release clean
	$(MAKE) -C $(RT28xx_DIR)/striptool -f Makefile.release
	striptool/striptool.out
ifeq ($(PLATFORM),MT85XX)
	cp -f tools/osabl_check.sh $(RELEASE)/
endif
ifeq ($(RELEASE), DPO)
	gcc -o striptool/banner striptool/banner.c
	./striptool/banner -b striptool/copyright.gpl -s DPO/ -d DPO_GPL -R
	./striptool/banner -b striptool/copyright.frm -s DPO_GPL/include/firmware.h
endif

prerelease:
ifeq ($(MODULE), 2880)
	$(MAKE) -C $(RT28xx_DIR)/os/linux -f Makefile.release.2880 prerelease
else
	$(MAKE) -C $(RT28xx_DIR)/os/linux -f Makefile.release prerelease
endif
	cp $(RT28xx_DIR)/os/linux/Makefile.DPB $(RTMP_SRC_DIR)/os/linux/.
	cp $(RT28xx_DIR)/os/linux/Makefile.DPA $(RTMP_SRC_DIR)/os/linux/.
	cp $(RT28xx_DIR)/os/linux/Makefile.DPC $(RTMP_SRC_DIR)/os/linux/.
ifeq ($(RT28xx_MODE),STA)
	cp $(RT28xx_DIR)/os/linux/Makefile.DPD $(RTMP_SRC_DIR)/os/linux/.
	cp $(RT28xx_DIR)/os/linux/Makefile.DPO $(RTMP_SRC_DIR)/os/linux/.
endif	

clean:
ifeq ($(TARGET), LINUX)
	cp -f os/linux/Makefile.clean os/linux/Makefile
	$(MAKE) -C os/linux clean
	rm -rf os/linux/Makefile
ifeq ($(PREALLOC), YES)
	cp -f PREALLOC/os/linux/Makefile.clean PREALLOC/os/linux/Makefile
	$(MAKE) -C PREALLOC/os/linux clean
	rm -rf PREALLOC/os/linux/Makefile
endif
endif	
ifeq ($(TARGET), UCOS)
	$(MAKE) -C os/ucos clean MODE=$(RT28xx_MODE)
endif
ifeq ($(TARGET), ECOS)
	$(MAKE) -C os/ecos clean MODE=$(RT28xx_MODE)
endif

uninstall:
ifeq ($(TARGET), LINUX)
ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	$(MAKE) -C $(RT28xx_DIR)/os/linux -f Makefile.4 uninstall
else
	$(MAKE) -C $(RT28xx_DIR)/os/linux -f Makefile.6 uninstall
endif
endif

install:
ifeq ($(TARGET), LINUX)
ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	$(MAKE) -C $(RT28xx_DIR)/os/linux -f Makefile.4 install
else
	$(MAKE) -C $(RT28xx_DIR)/os/linux -f Makefile.6 install
endif
endif

libwapi:
ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	cp -f os/linux/Makefile.libwapi.4 $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(RT28xx_DIR)/os/linux/
else
	cp -f os/linux/Makefile.libwapi.6 $(RT28xx_DIR)/os/linux/Makefile	
	$(MAKE) -C  $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules	
endif	

osutil:
ifeq ($(OSABL),YES)
ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	cp -f os/linux/Makefile.4.util $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(RT28xx_DIR)/os/linux/
else
	cp -f os/linux/Makefile.6.util $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
ifeq ($(PLATFORM),MT85XX)
	cp -f $(RT28xx_DIR)/os/linux/mt7603u_sta.ko $(RT28xx_DIR)/../../../../../../BDP_Generic/build_linux_ko/src/driver/wlan/
endif
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
ifeq ($(PLATFORM),MT85XX)
	cp -f $(RT28xx_DIR)/os/linux/mt7603u_sta.ko $(RT28xx_DIR)/../../../../../../BDP_Generic/build_linux_ko/src/driver/wlan/
endif
endif
endif

osdrv:
ifneq (,$(findstring 2.4,$(LINUX_SRC)))
	cp -f os/linux/Makefile.4 $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(RT28xx_DIR)/os/linux/
else
	cp -f os/linux/Makefile.6 $(RT28xx_DIR)/os/linux/Makefile
	$(MAKE) -C $(LINUX_SRC) SUBDIRS=$(RT28xx_DIR)/os/linux modules
ifeq ($(PLATFORM),MT85XX)
ifeq ($(OSABL),YES)
	cp -f $(RT28xx_DIR)/os/linux/mt7603u_sta.ko $(RT28xx_DIR)/../../../../../../BDP_Generic/build_linux_ko/src/driver/wlan/
endif
endif
endif

# Declare the contents of the .PHONY variable as phony.  We keep that information in a variable
.PHONY: $(PHONY)



