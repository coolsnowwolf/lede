#
# Copyright (C) 2016 Yousong Zhou <yszhou4tech@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
define KernelPackage/irqbypass
  SUBMENU:=Virtualization
  TITLE:=IRQ offload/bypass manager
  KCONFIG:=CONFIG_IRQ_BYPASS_MANAGER
  HIDDEN:=1
  FILES:= $(LINUX_DIR)/virt/lib/irqbypass.ko
  AUTOLOAD:=$(call AutoProbe,irqbypass.ko)
endef
$(eval $(call KernelPackage,irqbypass))


define KernelPackage/kvm-x86
  SUBMENU:=Virtualization
  TITLE:=Kernel-based Virtual Machine (KVM) support
  DEPENDS:=@TARGET_x86_generic||TARGET_x86_64 +kmod-irqbypass
  KCONFIG:=\
	CONFIG_KVM \
	CONFIG_KVM_MMU_AUDIT=n \
	CONFIG_VIRTUALIZATION=y
  FILES:= $(LINUX_DIR)/arch/$(LINUX_KARCH)/kvm/kvm.ko
  AUTOLOAD:=$(call AutoProbe,kvm.ko)
endef

define KernelPackage/kvm-x86/description
  Support hosting fully virtualized guest machines using hardware
  virtualization extensions.  You will need a fairly recent
  processor equipped with virtualization extensions. You will also
  need to select one or more of the processor modules.

  This module provides access to the hardware capabilities through
  a character device node named /dev/kvm.
endef

$(eval $(call KernelPackage,kvm-x86))


define KernelPackage/kvm-intel
  SUBMENU:=Virtualization
  TITLE:=KVM for Intel processors support
  DEPENDS:=+kmod-kvm-x86
  KCONFIG:=CONFIG_KVM_INTEL
  FILES:= $(LINUX_DIR)/arch/$(LINUX_KARCH)/kvm/kvm-intel.ko
  AUTOLOAD:=$(call AutoProbe,kvm-intel.ko)
endef

define KernelPackage/kvm-intel/description
  Provides support for KVM on Intel processors equipped with the VT
  extensions.
endef

$(eval $(call KernelPackage,kvm-intel))


define KernelPackage/kvm-amd
  SUBMENU:=Virtualization
  TITLE:=KVM for AMD processors support
  DEPENDS:=+kmod-kvm-x86
  KCONFIG:=CONFIG_KVM_AMD
  FILES:= $(LINUX_DIR)/arch/$(LINUX_KARCH)/kvm/kvm-amd.ko
  AUTOLOAD:=$(call AutoProbe,kvm-amd.ko)
endef

define KernelPackage/kvm-amd/description
  Provides support for KVM on AMD processors equipped with the AMD-V
  (SVM) extensions.
endef

$(eval $(call KernelPackage,kvm-amd))

define KernelPackage/vfio
  SUBMENU:=Virtualization
  TITLE:=VFIO Non-Privileged userspace driver framework
  DEPENDS:=@TARGET_x86_64
  KCONFIG:= \
	CONFIG_VFIO \
	CONFIG_VFIO_NOIOMMU=n \
	CONFIG_VFIO_MDEV=n
  MODPARAMS.vfio:=\
	enable_unsafe_noiommu_mode=n
  FILES:= \
	$(LINUX_DIR)/drivers/vfio/vfio.ko \
	$(LINUX_DIR)/drivers/vfio/vfio_virqfd.ko \
	$(LINUX_DIR)/drivers/vfio/vfio_iommu_type1.ko
  AUTOLOAD:=$(call AutoProbe,vfio vfio_iommu_type1 vfio_virqfd)
endef

define KernelPackage/vfio/description
  VFIO provides a framework for secure userspace device drivers.
endef

$(eval $(call KernelPackage,vfio))


define KernelPackage/vfio-pci
  SUBMENU:=Virtualization
  TITLE:=Generic VFIO support for any PCI device
  DEPENDS:=@TARGET_x86_64 @PCI_SUPPORT +kmod-vfio +kmod-irqbypass
  KCONFIG:= \
	CONFIG_VFIO_PCI \
	CONFIG_VFIO_PCI_IGD=n
  FILES:= \
	$(LINUX_DIR)/drivers/vfio/pci/vfio-pci-core.ko \
	$(LINUX_DIR)/drivers/vfio/pci/vfio-pci.ko
  AUTOLOAD:=$(call AutoProbe,vfio-pci)
endef

define KernelPackage/vfio-pci/description
  Support for the generic PCI VFIO bus driver which can connect any PCI
  device to the VFIO framework.
endef

$(eval $(call KernelPackage,vfio-pci))

define KernelPackage/iommu_v2
  SUBMENU:=Virtualization
  TITLE:=IOMMU Version 2 driver
  KCONFIG:=\
     CONFIG_UACCE=n \
     CONFIG_IOMMU_DEBUGFS=n \
     CONFIG_INTEL_IOMMU=y \
     CONFIG_INTEL_IOMMU_SVM=n \
     CONFIG_INTEL_IOMMU_DEFAULT_ON=n \
     CONFIG_INTEL_IOMMU_SCALABLE_MODE_DEFAULT_ON=n \
     CONFIG_INTEL_TXT=n \
     CONFIG_HYPERV_IOMMU=n \
     CONFIG_IOMMU_SUPPORT=y \
     CONFIG_IOMMU_DEFAULT_PASSTHROUGH=y \
     CONFIG_AMD_IOMMU=y \
     CONFIG_IRQ_REMAP=y \
     CONFIG_AMD_IOMMU_V2=m
  DEPENDS:= @PCI_SUPPORT @TARGET_x86_64
  FILES:= $(LINUX_DIR)/drivers/iommu/amd/iommu_v2.ko
  AUTOLOAD:=$(call AutoProbe,iommu_v2)
endef

define KernelPackage/iommu_v2/description
  This option enables support for the AMD/INTEL IOMMUv2 features
  of the IOMMU hardware. Select this option if you want
  to use devices that support the PCI PRI and PASID interface.
endef

$(eval $(call KernelPackage,iommu_v2))

define KernelPackage/vfio-mdev
  SUBMENU:=Virtualization
  TITLE:=VFIO driver support to to virtualize devices
  DEPENDS:=@TARGET_x86_64
  KCONFIG:=	\
	CONFIG_IOMMU_API=y \
	CONFIG_MMU=y \
	CONFIG_VFIO=y \
	CONFIG_VFIO_MDEV \
	CONFIG_VFIO_MDEV_DEVICE \
	CONFIG_VFIO_NOIOMMU=y \
	CONFIG_VFIO_PCI=y \
	CONFIG_VFIO_PCI_IGD=y
  FILES:= \
	$(LINUX_DIR)/drivers/vfio/mdev/mdev.ko \
	$(LINUX_DIR)/drivers/vfio/mdev/vfio_mdev.ko@lt5.10
  AUTOLOAD:=$(call AutoProbe,mdev vfio_mdev)
endef

define KernelPackage/vfio-mdev/description
  Provides a framework to virtualize devices.
endef

$(eval $(call KernelPackage,vfio-mdev))

define KernelPackage/i915-gvt
  SUBMENU:=Virtualization
  TITLE:=Enable KVM/VFIO support for Intel GVT-g
  DEPENDS:=@TARGET_x86_64 +kmod-kvm-intel +kmod-drm-i915 +kmod-vfio-mdev
  KCONFIG:= CONFIG_DRM_I915_GVT_KVMGT
  FILES:= \
      $(LINUX_DIR)/drivers/gpu/drm/i915/gvt/kvmgt.ko@lt5.18 \
      $(LINUX_DIR)/drivers/gpu/drm/i915/kvmgt.ko@ge5.18
  AUTOLOAD:=$(call AutoProbe,kvmgt)
endef

define KernelPackage/i915-gvt/description
  Enable Intel GVT-g graphics virtualization technology host support with 
  integrated graphics. With GVT-g, it's possible to have one integrated 
  graphics device shared by multiple VMs under KVM.
endef

$(eval $(call KernelPackage,i915-gvt))
