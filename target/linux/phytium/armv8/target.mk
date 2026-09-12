# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2023-2024 Ailick <277498654@qq.com>

ARCH:=aarch64
SUBTARGET:=armv8
BOARDNAME:=64-bit (armv8) SystemReady (EFI) compliant
FEATURES+=display

DEFAULT_PACKAGES += kmod-phytium-dc kmod-phytium-gpu kmod-phytium-vpu kmod-phytium-npu

define Target/Description
  Build multi-platform images for the Phytium ARMv8 instruction set architecture
endef
