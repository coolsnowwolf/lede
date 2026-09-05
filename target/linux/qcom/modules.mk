# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2026

define KernelPackage/qcom-mdt-loader
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Qualcomm MDT firmware loader helper
  HIDDEN:=1
  DEPENDS:=@TARGET_qcom
  KCONFIG:=CONFIG_QCOM_MDT_LOADER
  FILES:= \
	$(LINUX_DIR)/drivers/soc/qcom/mdt_loader.ko
endef

define KernelPackage/qcom-mdt-loader/description
  Helper module for loading Qualcomm MDT firmware images
endef

$(eval $(call KernelPackage,qcom-mdt-loader))

define KernelPackage/qcom-ubwc-config
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Qualcomm UBWC configuration helper
  HIDDEN:=1
  DEPENDS:=@TARGET_qcom
  KCONFIG:=CONFIG_QCOM_UBWC_CONFIG=m
  FILES:= \
	$(LINUX_DIR)/drivers/soc/qcom/ubwc_config.ko
  AUTOLOAD:=$(call AutoProbe,ubwc_config)
endef

define KernelPackage/qcom-ubwc-config/description
  Helper module for Qualcomm Universal Bandwidth Compression configuration
endef

$(eval $(call KernelPackage,qcom-ubwc-config))

define KernelPackage/drm-simple-bridge
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Simple DRM bridge support
  HIDDEN:=1
  DEPENDS:=@TARGET_qcom +kmod-drm
  KCONFIG:=CONFIG_DRM_SIMPLE_BRIDGE=m
  FILES:= \
	$(LINUX_DIR)/drivers/gpu/drm/bridge/simple-bridge.ko
  AUTOLOAD:=$(call AutoProbe,simple-bridge)
endef

define KernelPackage/drm-simple-bridge/description
  Simple DRM bridge helper used by transparent DP/HDMI bridge chips.
endef

$(eval $(call KernelPackage,drm-simple-bridge))

define KernelPackage/drm-display-connector
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=DRM display connector bridge support
  HIDDEN:=1
  DEPENDS:=@TARGET_qcom +kmod-drm
  KCONFIG:=CONFIG_DRM_DISPLAY_CONNECTOR=m
  FILES:= \
	$(LINUX_DIR)/drivers/gpu/drm/bridge/display-connector.ko
  AUTOLOAD:=$(call AutoProbe,display-connector)
endef

define KernelPackage/drm-display-connector/description
  DRM bridge driver for OF graph HDMI/DP/DVI/VGA display connector nodes.
endef

$(eval $(call KernelPackage,drm-display-connector))

define KernelPackage/drm-msm
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Qualcomm MSM DRM support
  DEPENDS:=@TARGET_qcom +kmod-backlight +kmod-drm-kms-helper \
	+kmod-drm-display-helper +kmod-drm-client-lib +kmod-drm-sched \
	+kmod-drm-gem-shmem-helper +kmod-qcom-mdt-loader \
	+kmod-qcom-ubwc-config +kmod-drm-simple-bridge \
	+kmod-drm-display-connector \
	+LINUX_6_18:kmod-drm-exec \
	+LINUX_6_18:kmod-drm-suballoc-helper
  KCONFIG:= \
	CONFIG_DRM_MSM \
	CONFIG_DRM_MSM_DP \
	CONFIG_DRM_GPUVM \
	CONFIG_QCOM_UBWC_CONFIG \
	CONFIG_DRM_MSM_DPU \
	CONFIG_DRM_MSM_KMS \
	CONFIG_DRM_MSM_KMS_FBDEV \
	CONFIG_DRM_MSM_MDSS \
	CONFIG_DRM_MSM_GPU_STATE=y
  FILES:= \
	$(LINUX_DIR)/drivers/gpu/drm/display/drm_dp_aux_bus.ko \
	$(LINUX_DIR)/drivers/gpu/drm/drm_gpuvm.ko \
	$(LINUX_DIR)/drivers/gpu/drm/msm/msm.ko
  AUTOLOAD:=$(call AutoProbe,msm)
endef

define KernelPackage/drm-msm/description
  Direct Rendering Manager (DRM) support for Qualcomm MSM display and Adreno GPU blocks
endef

$(eval $(call KernelPackage,drm-msm))

define KernelPackage/qcom-videocc
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Qualcomm video clock controller support
  DEPENDS:=@TARGET_qcom
  KCONFIG:=CONFIG_SM_VIDEOCC_8350
  FILES:= \
	$(LINUX_DIR)/drivers/clk/qcom/videocc-sm8350.ko
  AUTOLOAD:=$(call AutoProbe,videocc-sm8350)
endef

define KernelPackage/qcom-videocc/description
  Qualcomm SM8350/SC8280XP class video clock controller support
endef

$(eval $(call KernelPackage,qcom-videocc))

define KernelPackage/video-qcom-venus
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Qualcomm Venus VPU support
  DEPENDS:=@TARGET_qcom +kmod-qcom-mdt-loader +kmod-video-mem2mem \
	+kmod-video-dma +qcom-venus-firmware
  KCONFIG:=CONFIG_VIDEO_QCOM_VENUS
  FILES:= \
	$(LINUX_DIR)/drivers/media/platform/qcom/venus/venus-core.ko \
	$(LINUX_DIR)/drivers/media/platform/qcom/venus/venus-dec.ko \
	$(LINUX_DIR)/drivers/media/platform/qcom/venus/venus-enc.ko
  AUTOLOAD:=$(call AutoProbe,venus-core venus-dec venus-enc)
  $(call AddDepends/video)
endef

define KernelPackage/video-qcom-venus/description
  Qualcomm Venus hardware video encode and decode accelerator support
endef

$(eval $(call KernelPackage,video-qcom-venus))

define KernelPackage/video-qcom-iris
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Qualcomm Iris VPU support
  DEPENDS:=@TARGET_qcom +kmod-qcom-mdt-loader +kmod-video-mem2mem \
	+kmod-video-dma +kmod-qcom-videocc +kmod-qcom-ubwc-config \
	+qcom-iris-firmware
  KCONFIG:=CONFIG_VIDEO_QCOM_IRIS
  FILES:= \
	$(LINUX_DIR)/drivers/media/platform/qcom/iris/qcom-iris.ko
  AUTOLOAD:=$(call AutoProbe,qcom-iris)
  $(call AddDepends/video)
endef

define KernelPackage/video-qcom-iris/description
  Qualcomm Iris hardware video encode and decode accelerator support
endef

$(eval $(call KernelPackage,video-qcom-iris))

define KernelPackage/qcom-fastrpc
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Qualcomm FastRPC support
  DEPENDS:=@TARGET_qcom +kmod-dma-buf
  KCONFIG:= \
	CONFIG_QCOM_FASTRPC \
	CONFIG_DMA_SHARED_BUFFER \
	CONFIG_QCOM_SCM
  FILES:= \
	$(LINUX_DIR)/drivers/misc/fastrpc.ko
  AUTOLOAD:=$(call AutoProbe,fastrpc)
endef

define KernelPackage/qcom-fastrpc/description
  Qualcomm FastRPC userspace interface for invoking services on ADSP/CDSP/NSP remote processors
endef

$(eval $(call KernelPackage,qcom-fastrpc))

define KernelPackage/npu-qcom
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Qualcomm NSP/NPU remote processor support
  DEPENDS:=@TARGET_qcom +kmod-qcom-mdt-loader +kmod-qcom-fastrpc
  KCONFIG:= \
	CONFIG_REMOTEPROC \
	CONFIG_REMOTEPROC_CDEV \
	CONFIG_RPMSG \
	CONFIG_QCOM_PIL_INFO \
	CONFIG_QCOM_RPROC_COMMON \
	CONFIG_QCOM_Q6V5_COMMON \
	CONFIG_QCOM_Q6V5_ADSP \
	CONFIG_QCOM_Q6V5_PAS \
	CONFIG_QCOM_SYSMON \
	CONFIG_RPMSG_QCOM_GLINK \
	CONFIG_RPMSG_CHAR \
	CONFIG_RPMSG_CTRL \
	CONFIG_RPMSG_QCOM_GLINK_SMEM
  FILES:= \
	$(LINUX_DIR)/drivers/remoteproc/qcom_q6v5_adsp.ko \
	$(LINUX_DIR)/drivers/remoteproc/qcom_q6v5.ko \
	$(LINUX_DIR)/drivers/remoteproc/qcom_common.ko \
	$(LINUX_DIR)/drivers/remoteproc/qcom_pil_info.ko \
	$(LINUX_DIR)/drivers/remoteproc/qcom_q6v5_pas.ko \
	$(LINUX_DIR)/drivers/remoteproc/qcom_sysmon.ko \
	$(LINUX_DIR)/drivers/rpmsg/qcom_glink_smem.ko \
	$(LINUX_DIR)/drivers/rpmsg/rpmsg_char.ko \
	$(LINUX_DIR)/drivers/rpmsg/rpmsg_ctrl.ko
  AUTOLOAD:=$(call AutoProbe,qcom_q6v5_adsp qcom_q6v5_pas qcom_sysmon qcom_glink_smem rpmsg_char rpmsg_ctrl)
endef

define KernelPackage/npu-qcom/description
  Qualcomm NSP/NPU related remoteproc and RPMsg support for SC8280XP class platforms
endef

$(eval $(call KernelPackage,npu-qcom))

define KernelPackage/qcom-qaic
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Qualcomm QAIC accelerator support
  DEPENDS:=@TARGET_qcom +kmod-drm +kmod-mhi-bus
  KCONFIG:= \
	CONFIG_DRM_ACCEL=y \
	CONFIG_DRM_ACCEL_QAIC=m \
	CONFIG_CRC32 \
	CONFIG_WANT_DEV_COREDUMP
  FILES:= \
	$(LINUX_DIR)/drivers/accel/qaic/qaic.ko
  AUTOLOAD:=$(call AutoProbe,qaic)
endef

define KernelPackage/qcom-qaic/description
  Qualcomm AI accelerator DRM accel driver used by QAIC/MHI based NPU devices
endef

$(eval $(call KernelPackage,qcom-qaic))

define KernelPackage/tc956x-pci
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Toshiba TC956x / Qualcomm QPS615 PCI bridge support
  DEPENDS:=@TARGET_qcom +kmod-i2c-core +kmod-regmap-i2c
  KCONFIG:=CONFIG_TOSHIBA_TC956X_PCI
  FILES:=$(LINUX_DIR)/drivers/misc/tc956x_pci.ko
  AUTOLOAD:=$(call AutoProbe,tc956x_pci)
endef

define KernelPackage/tc956x-pci/description
  Kernel support for the Toshiba TC956x / Qualcomm QPS615 PCIe bridge
endef

$(eval $(call KernelPackage,tc956x-pci))

define KernelPackage/stmmac-tc956x
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Toshiba TC956x/QPS615 Ethernet support
  DEPENDS:=@TARGET_qcom +kmod-stmmac-core +kmod-libphy +kmod-mdio-devres +kmod-ptp +kmod-tc956x-pci
  KCONFIG:=CONFIG_TOSHIBA_TC956X_PCI CONFIG_DWMAC_TC956X
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/stmicro/stmmac/dwmac-tc956x.ko
  AUTOLOAD:=$(call AutoProbe,dwmac-tc956x)
endef

define KernelPackage/stmmac-tc956x/description
  Kernel support for the Toshiba TC956x / Qualcomm QPS615 Ethernet bridge
endef

$(eval $(call KernelPackage,stmmac-tc956x))
