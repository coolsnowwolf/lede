# SPDX-License-Identifier: GPL-2.0

# Out-of-tree Makefile rules

M ?= $(shell pwd)

KBASE_PATH_RELATIVE = $(M)

ifndef CONFIG_DRM_PHYTIUM
# When building out of tree default to build as module
CONFIG_DRM_PHYTIUM = m

# apply out-of-tree build 9x00 defaults
CONFIG_PHYTIUM_CHIP_FTD330 ?= y
CONFIG_PHYTIUM_WRITEBACK ?= n
CONFIG_PHYTIUM_DEC ?= y
CONFIG_PHYTIUM_MMU ?= y
CONFIG_PHYTIUM_NCC	?= y
CONFIG_PHYTIUM_CHANGE_PIXCLK ?= y
CONFIG_PHYTIUM_FBDEV_ON ?= y
CONFIG_PHYTIUM_LANE_TRAIN ?= y
CONFIG_PHYTIUM_POWER_OPERATION ?= y
CONFIG_PHYTIUM_PCIE	?= n
CONFIG_PHYTIUM_EDP_BL  ?= y
CONFIG_PHYTIUM_PSR  ?= y
CONFIG_PHYTIUM_PSR_SF_UPDATE  ?= n
CONFIG_PHYTIUM_LOW_FPS  ?= y

# phytium internal config
CONFIG_PHYTIUM_PCIE_GEN7 ?= n
CONFIG_PHYTIUM_VIRTUAL_DISPLAY ?= n

KBUILD_OPTIONS += CONFIG_DRM_PHYTIUM=$(CONFIG_DRM_PHYTIUM)

PHYTIUM_CONFIGS := CHIP_FTD330 \
                       DCFTD330_0x31b \
                       DCFTD330_0x311 \
                       DCFTD330_0x310 \
                       DCFTD330_0x30b \
                       DCFTD330_0x331 \
                       DCFTD330_0x335 \
		       MMU \
                       DEC \
                       WRITEBACK \
                       PCIE \
                       VIRTUAL_DISPLAY \
		       NCC	\
		       CHANGE_PIXCLK	\
		       FBDEV_ON		\
		       LANE_TRAIN	\
		       POWER_OPERATION  \
                       EDP_BL           \
                       PSR              \
                       PSR_SF_UPDATE    \
                       LOW_FPS          \

KBUILD_OPTIONS += $(foreach c,$(PHYTIUM_CONFIGS), \
                  CONFIG_PHYTIUM_$(c)=$(value $(addsuffix $(c),CONFIG_PHYTIUM_)))

EXTRA_CFLAGS += $(foreach c,$(PHYTIUM_CONFIGS), \
                $(if $(filter y,$(value $(addsuffix $(c),CONFIG_PHYTIUM_))),-DCONFIG_PHYTIUM_$(c)))
endif

modules modules_install headers_install clean:
		$(MAKE) -C $(KERNEL_SRC) M=$(M) W=1 $(KBUILD_OPTIONS) \
                EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
                KBUILD_EXTRA_SYMBOLS="$(EXTRA_SYMBOLS)" $(@)

modules_install: headers_install
