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
