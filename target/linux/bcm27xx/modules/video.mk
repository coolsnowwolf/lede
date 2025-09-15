# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2019 OpenWrt.org

define KernelPackage/camera-bcm2835
  TITLE:=BCM2835 Camera
  KCONFIG:= \
    CONFIG_VIDEO_BCM2835
  FILES:= \
    $(LINUX_DIR)/drivers/staging/vc04_services/bcm2835-camera/bcm2835-v4l2.ko
  AUTOLOAD:=$(call AutoLoad,66,bcm2835-v4l2)
  $(call AddDepends/video,@TARGET_bcm27xx +kmod-vchiq-mmal-bcm2835 +kmod-video-videobuf2)
endef

define KernelPackage/camera-bcm2835/description
  Camera host interface devices for Broadcom BCM2835 SoC.
  This operates over the VCHIQ interface to a service running on VideoCore.
endef

$(eval $(call KernelPackage,camera-bcm2835))


define KernelPackage/rp1-cfe
  TITLE:=RP1 Camera Front-End
  SUBMENU:=$(VIDEO_MENU)
  KCONFIG:= \
     CONFIG_VIDEO_RP1_CFE \
     CONFIG_VIDEO_BCM2835
  FILES:=$(LINUX_DIR)/drivers/media/platform/raspberrypi/rp1_cfe/rp1-cfe.ko
  AUTOLOAD:=$(call AutoLoad,67,rp1-cfe)
  DEPENDS:=@TARGET_bcm27xx_bcm2712 +kmod-video-core +kmod-video-fwnode +kmod-video-dma-contig +kmod-video-async
endef

define KernelPackage/rp1-cfe/description
  Driver for the Camera Serial Interface (CSI) to capture video
  streams from connected cameras.
endef

$(eval $(call KernelPackage,rp1-cfe))


define KernelPackage/codec-bcm2835
  TITLE:=BCM2835 Video Codec
  KCONFIG:= \
    CONFIG_VIDEO_CODEC_BCM2835
  FILES:= \
    $(LINUX_DIR)/drivers/staging/vc04_services/bcm2835-codec/bcm2835-codec.ko
  AUTOLOAD:=$(call AutoLoad,67,bcm2835-codec)
  $(call AddDepends/video,@TARGET_bcm27xx +kmod-vchiq-mmal-bcm2835 +kmod-video-dma-contig +kmod-video-mem2mem)
endef

define KernelPackage/codec-bcm2835/description
  V4L2 video codecs for Broadcom BCM2835 SoC.
  This operates over the VCHIQ interface to a service running on VideoCore.
endef

$(eval $(call KernelPackage,codec-bcm2835))


define KernelPackage/drm-vc4
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Broadcom VC4 Graphics
  DEPENDS:= \
    @TARGET_bcm27xx +kmod-drm \
    +kmod-sound-core \
    +kmod-sound-soc-core
  KCONFIG:= \
    CONFIG_DRM_VC4 \
    CONFIG_DRM_VC4_HDMI_CEC=y
  FILES:= \
    $(LINUX_DIR)/drivers/gpu/drm/display/drm_display_helper.ko \
    $(LINUX_DIR)/drivers/gpu/drm/drm_dma_helper.ko \
    $(LINUX_DIR)/drivers/gpu/drm/vc4/vc4.ko \
    $(LINUX_DIR)/drivers/gpu/drm/drm_kms_helper.ko \
    $(LINUX_DIR)/drivers/media/cec/core/cec.ko
  AUTOLOAD:=$(call AutoProbe,vc4)
endef

define KernelPackage/drm-vc4/description
  Direct Rendering Manager (DRM) support for Broadcom VideoCore IV GPU
  used in BCM2835, BCM2836 and BCM2837 SoCs (e.g. Raspberry Pi).
endef

$(eval $(call KernelPackage,drm-vc4))


define KernelPackage/isp-bcm2835
  TITLE:=BCM2835 ISP
  KCONFIG:= \
    CONFIG_VIDEO_ISP_BCM2835
  FILES:= \
    $(LINUX_DIR)/drivers/staging/vc04_services/bcm2835-isp/bcm2835-isp.ko
  AUTOLOAD:=$(call AutoLoad,67,bcm2835-isp)
  $(call AddDepends/video,@TARGET_bcm27xx +kmod-vchiq-mmal-bcm2835 +kmod-video-dma-contig)
endef

define KernelPackage/isp-bcm2835/description
  V4L2 driver for the Broadcom BCM2835 ISP hardware.
  This operates over the VCHIQ interface to a service running on VideoCore.
endef

$(eval $(call KernelPackage,isp-bcm2835))


define KernelPackage/vc-sm-cma
  TITLE:=VideoCore Shared Memory (CMA) driver
  KCONFIG:= \
    CONFIG_BCM_VC_SM_CMA
  FILES:= \
    $(LINUX_DIR)/drivers/staging/vc04_services/vc-sm-cma/vc-sm-cma.ko
  $(call AddDepends/video,@TARGET_bcm27xx)
endef

define KernelPackage/vc-sm-cma/description
  Shared memory interface that supports sharing dmabufs with VideoCore.
  This operates over the VCHIQ interface to a service running on VideoCore.
endef

$(eval $(call KernelPackage,vc-sm-cma))


define KernelPackage/vchiq-mmal-bcm2835
  TITLE:=BCM2835 MMAL VCHIQ service
  KCONFIG:= \
    CONFIG_BCM2835_VCHIQ_MMAL
  FILES:= \
    $(LINUX_DIR)/drivers/staging/vc04_services/vchiq-mmal/bcm2835-mmal-vchiq.ko
  $(call AddDepends/video,@TARGET_bcm27xx +kmod-vc-sm-cma)
endef

define KernelPackage/vchiq-mmal-bcm2835/description
  Enables the MMAL API over VCHIQ as used for the
  majority of the multimedia services on VideoCore.
endef

$(eval $(call KernelPackage,vchiq-mmal-bcm2835))


define KernelPackage/drm-rp1-dsi
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=RP1 Display Serial Interface for Video
  KCONFIG:= \
    CONFIG_DRM_RP1_DSI \
    CONFIG_DRM_MIPI_DSI=y \
    CONFIG_GENERIC_PHY_MIPI_DPHY=n \
    CONFIG_DRM_WERROR=n
  FILES:=$(LINUX_DIR)/drivers/gpu/drm/rp1/rp1-dsi/drm-rp1-dsi.ko
  AUTOLOAD:=$(call AutoLoad,67,drm-rp1-dsi)
  DEPENDS:=@TARGET_bcm27xx_bcm2712 +kmod-drm-vc4 \
    +kmod-drm-dma-helper +kmod-drm-vram-helper
endef

define KernelPackage/drm-rp1-dsi/description
  This module manages the DSI for driving high-resolution LCD panels
  such as the official Raspberry Pi displays or other screens that
  use the DSI interface.
endef

$(eval $(call KernelPackage,drm-rp1-dsi))


define KernelPackage/drm-rp1-dpi
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=RP1 Display Parallel Interface for Video
  KCONFIG:= \
    CONFIG_DRM_RP1_DPI
  FILES:=$(LINUX_DIR)/drivers/gpu/drm/rp1/rp1-dpi/drm-rp1-dpi.ko
  AUTOLOAD:=$(call AutoLoad,67,drm-rp1-dpi)
  DEPENDS:=@TARGET_bcm27xx_bcm2712 +kmod-drm-vc4 \
    +kmod-drm-dma-helper +kmod-drm-vram-helper \
    +kmod-rp1-pio
endef

define KernelPackage/drm-rp1-dpi/description
  This module is or driving displays using the DPI standard.
  Useful for interfacing with custom or low-level LCD panels
  that require parallel RGB signals.  Provides direct control
  over the timing and signal driving of raw LCD panels.
  Typically used in maker projects or with non-HDMI displays.
endef

$(eval $(call KernelPackage,drm-rp1-dpi))


define KernelPackage/drm-rp1-vec
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=RP1 Display Composite Video
  KCONFIG:= \
    CONFIG_DRM_RP1_VEC
  FILES:=$(LINUX_DIR)/drivers/gpu/drm/rp1/rp1-vec/drm-rp1-vec.ko
  AUTOLOAD:=$(call AutoLoad,67,drm-rp1-vec)
  DEPENDS:=@TARGET_bcm27xx_bcm2712 +kmod-drm-vc4 \
    +kmod-drm-dma-helper +kmod-drm-vram-helper
endef

define KernelPackage/drm-rp1-vec/description
  This module is used for composite video output, which is typically
  transmitted through the RCA jack.  Primary use is onnecting older
  TVs or monitors that rely on analog signals via a composite interface.
  Handles standard-definition analog signals in NTSC and PAL.
endef

$(eval $(call KernelPackage,drm-rp1-vec))
