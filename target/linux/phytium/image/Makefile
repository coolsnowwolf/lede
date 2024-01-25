# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2016-2017 Yousong Zhou <yszhou4tech@gmail.com>
# Copyright (c) 2023 Phytium Technology Co., Ltd.
# Copyright (C) 2023-2024 Ailick <277498654@qq.com>

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Build/Compile
	$(CP) $(LINUX_DIR)/COPYING $(KDIR)/COPYING.linux
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))
