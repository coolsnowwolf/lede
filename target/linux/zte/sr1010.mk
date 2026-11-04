# SPDX-License-Identifier: GPL-2.0-only
# Source blobs are kept together outside the device-tree directory.
SR1010_FIRMWARE_DIR := $(TOPDIR)/package/firmware/zte/flies
SR1010_OPENSSL := $(if $(wildcard $(STAGING_DIR_HOST)/bin/openssl),$(STAGING_DIR_HOST)/bin/openssl,openssl)
