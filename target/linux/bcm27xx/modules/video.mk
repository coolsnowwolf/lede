# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2019 OpenWrt.org

define KernelPackage/camera-bcm2835
  TITLE:=BCM2835 Camera
  KCONFIG:= \
    CONFIG_VIDEO_BCM2835 \
    CONFIG_VIDEO_BCM2835_MMAL \
    CONFIG_VIDEO_BCM2835_UNICAM=n \
    CONFIG_VIDEO_ISP_BCM2835=n
  FILES:= \
    $(LINUX_DIR)/drivers/staging/vc04_services/bcm2835-camera/bcm2835-v4l2.ko
  AUTOLOAD:=$(call AutoLoad,65,bcm2835-v4l2)
  $(call AddDepends/video,@TARGET_bcm27xx +kmod-vchiq-mmal-bcm2835 +kmod-video-videobuf2)
endef

define KernelPackage/camera-bcm2835/description
  Camera host interface devices for Broadcom BCM2835 SoC.
  This operates over the VCHIQ interface to a service running on VideoCore.
endef

$(eval $(call KernelPackage,camera-bcm2835))

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
    CONFIG_BCM2835_VCHIQ_MMAL \
    CONFIG_VIDEO_CODEC_BCM2835=n
  FILES:= \
    $(LINUX_DIR)/drivers/staging/vc04_services/vchiq-mmal/bcm2835-mmal-vchiq.ko
  $(call AddDepends/video,@TARGET_bcm27xx +kmod-vc-sm-cma)
endef

define KernelPackage/vchiq-mmal-bcm2835/description
  Enables the MMAL API over VCHIQ as used for the
  majority of the multimedia services on VideoCore.
endef

$(eval $(call KernelPackage,vchiq-mmal-bcm2835))

define KernelPackage/drm-vc4
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Broadcom VC4 Graphics
  DEPENDS:= \
    @TARGET_bcm27xx +kmod-drm \
    +kmod-sound-core \
    +kmod-sound-soc-core \
    +kmod-drm-kms-helper 
  KCONFIG:= \
    CONFIG_DRM_VC4 \
    CONFIG_DRM_VC4_HDMI_CEC=y \
    CONFIG_DRM_GUD=n \
    CONFIG_DRM_V3D=n \
    CONFIG_DRM_TVE200=n
  FILES:= \
    $(LINUX_DIR)/drivers/gpu/drm/vc4/vc4.ko \
    $(LINUX_DIR)/drivers/media/cec/cec.ko@lt5.10 \
    $(LINUX_DIR)/drivers/media/cec/core/cec.ko@ge5.10
  AUTOLOAD:=$(call AutoProbe,vc4)
endef

define KernelPackage/drm-vc4/description
  Direct Rendering Manager (DRM) support for Broadcom VideoCore IV GPU
  used in BCM2835, BCM2836 and BCM2837 SoCs (e.g. Raspberry Pi).
endef

$(eval $(call KernelPackage,drm-vc4))
