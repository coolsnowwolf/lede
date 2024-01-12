#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

LIB_MENU:=Libraries

define KernelPackage/lib-crc-ccitt
  SUBMENU:=$(LIB_MENU)
  TITLE:=CRC-CCITT support
  KCONFIG:=CONFIG_CRC_CCITT
  FILES:=$(LINUX_DIR)/lib/crc-ccitt.ko
  AUTOLOAD:=$(call AutoProbe,crc-ccitt)
endef

define KernelPackage/lib-crc-ccitt/description
 Kernel module for CRC-CCITT support
endef

$(eval $(call KernelPackage,lib-crc-ccitt))


define KernelPackage/lib-crc-itu-t
  SUBMENU:=$(LIB_MENU)
  TITLE:=CRC ITU-T V.41 support
  KCONFIG:=CONFIG_CRC_ITU_T
  FILES:=$(LINUX_DIR)/lib/crc-itu-t.ko
  AUTOLOAD:=$(call AutoProbe,crc-itu-t)
endef

define KernelPackage/lib-crc-itu-t/description
 Kernel module for CRC ITU-T V.41 support
endef

$(eval $(call KernelPackage,lib-crc-itu-t))


define KernelPackage/lib-crc7
  SUBMENU:=$(LIB_MENU)
  TITLE:=CRC7 support
  KCONFIG:=CONFIG_CRC7
  FILES:=$(LINUX_DIR)/lib/crc7.ko
  AUTOLOAD:=$(call AutoProbe,crc7)
endef

define KernelPackage/lib-crc7/description
 Kernel module for CRC7 support
endef

$(eval $(call KernelPackage,lib-crc7))


define KernelPackage/lib-crc8
  SUBMENU:=$(LIB_MENU)
  TITLE:=CRC8 support
  KCONFIG:=CONFIG_CRC8
  FILES:=$(LINUX_DIR)/lib/crc8.ko
  AUTOLOAD:=$(call AutoProbe,crc8)
endef

define KernelPackage/lib-crc8/description
 Kernel module for CRC8 support
endef

$(eval $(call KernelPackage,lib-crc8))


define KernelPackage/lib-crc16
  SUBMENU:=$(LIB_MENU)
  TITLE:=CRC16 support
  KCONFIG:=CONFIG_CRC16
  FILES:=$(LINUX_DIR)/lib/crc16.ko
  AUTOLOAD:=$(call AutoLoad,20,crc16,1)
endef

define KernelPackage/lib-crc16/description
 Kernel module for CRC16 support
endef

$(eval $(call KernelPackage,lib-crc16))


define KernelPackage/lib-crc32c
  SUBMENU:=$(LIB_MENU)
  TITLE:=CRC32 support
  KCONFIG:=CONFIG_LIBCRC32C
  DEPENDS:=+kmod-crypto-crc32c
  FILES:=$(LINUX_DIR)/lib/libcrc32c.ko
  AUTOLOAD:=$(call AutoProbe,libcrc32c)
endef

define KernelPackage/lib-crc32c/description
 Kernel module for CRC32 support
endef

$(eval $(call KernelPackage,lib-crc32c))


define KernelPackage/lib-lzo
  SUBMENU:=$(LIB_MENU)
  TITLE:=LZO support
  DEPENDS:=+kmod-crypto-acompress
  KCONFIG:= \
	CONFIG_CRYPTO_LZO \
	CONFIG_LZO_COMPRESS \
	CONFIG_LZO_DECOMPRESS
  HIDDEN:=1
  FILES:= \
	$(LINUX_DIR)/crypto/lzo.ko \
	$(LINUX_DIR)/crypto/lzo-rle.ko \
	$(LINUX_DIR)/lib/lzo/lzo_compress.ko \
	$(LINUX_DIR)/lib/lzo/lzo_decompress.ko
  AUTOLOAD:=$(call AutoProbe,lzo lzo-rle lzo_compress lzo_decompress)
endef

define KernelPackage/lib-lzo/description
 Kernel module for LZO compression/decompression support
endef

$(eval $(call KernelPackage,lib-lzo))


define KernelPackage/lib-zstd
  SUBMENU:=$(LIB_MENU)
  TITLE:=ZSTD support
  DEPENDS:=+kmod-crypto-acompress
  KCONFIG:= \
	CONFIG_CRYPTO_ZSTD \
	CONFIG_ZSTD_COMPRESS \
	CONFIG_ZSTD_DECOMPRESS \
	CONFIG_XXHASH
  FILES:= \
	$(LINUX_DIR)/crypto/zstd.ko \
	$(LINUX_DIR)/lib/xxhash.ko \
	$(LINUX_DIR)/lib/zstd/zstd_common.ko@ge6.1 \
	$(LINUX_DIR)/lib/zstd/zstd_compress.ko \
	$(LINUX_DIR)/lib/zstd/zstd_decompress.ko
  AUTOLOAD:=$(call AutoProbe,xxhash zstd zstd_compress zstd_decompress)
endef

define KernelPackage/lib-zstd/description
 Kernel module for ZSTD compression/decompression support
endef

$(eval $(call KernelPackage,lib-zstd))


define KernelPackage/lib-lz4
  SUBMENU:=$(LIB_MENU)
  TITLE:=LZ4 support
  DEPENDS:=+kmod-crypto-acompress
  KCONFIG:= \
	CONFIG_CRYPTO_LZ4 \
	CONFIG_CRYPTO_LZ4HC \
	CONFIG_LZ4_COMPRESS \
	CONFIG_LZ4_DECOMPRESS
  FILES:= \
	$(LINUX_DIR)/crypto/lz4.ko \
	$(LINUX_DIR)/lib/lz4/lz4_compress.ko \
	$(LINUX_DIR)/lib/lz4/lz4hc_compress.ko \
	$(LINUX_DIR)/lib/lz4/lz4_decompress.ko
  AUTOLOAD:=$(call AutoProbe,lz4 lz4_compress lz4hc_compress lz4_decompress)
endef

define KernelPackage/lib-lz4/description
 Kernel module for LZ4 compression/decompression support
endef

$(eval $(call KernelPackage,lib-lz4))


define KernelPackage/lib-842
  SUBMENU:=$(LIB_MENU)
  TITLE:=842 support
  DEPENDS:=+kmod-crypto-acompress +kmod-crypto-crc32
  KCONFIG:= \
	CONFIG_CRYPTO_842 \
	CONFIG_842_COMPRESS \
	CONFIG_842_DECOMPRESS
  FILES:= \
	$(LINUX_DIR)/crypto/842.ko \
	$(LINUX_DIR)/lib/842/842_compress.ko \
	$(LINUX_DIR)/lib/842/842_decompress.ko
  AUTOLOAD:=$(call AutoProbe,842 842_compress 842_decompress)
endef

define KernelPackage/lib-842/description
 Kernel module for 842 compression/decompression support
endef

$(eval $(call KernelPackage,lib-842))


define KernelPackage/lib-raid6
  SUBMENU:=$(LIB_MENU)
  TITLE:=RAID6 algorithm support
  HIDDEN:=1
  KCONFIG:=CONFIG_RAID6_PQ
  FILES:=$(LINUX_DIR)/lib/raid6/raid6_pq.ko
  AUTOLOAD:=$(call AutoProbe,raid6_pq)
endef

define KernelPackage/lib-raid6/description
 Kernel module for RAID6 algorithms
endef

$(eval $(call KernelPackage,lib-raid6))


define KernelPackage/lib-xor
  SUBMENU:=$(LIB_MENU)
  TITLE:=XOR blocks algorithm support
  HIDDEN:=1
  KCONFIG:=CONFIG_XOR_BLOCKS
ifneq ($(wildcard $(LINUX_DIR)/arch/$(LINUX_KARCH)/lib/xor-neon.ko),)
  FILES:= \
    $(LINUX_DIR)/crypto/xor.ko \
    $(LINUX_DIR)/arch/$(LINUX_KARCH)/lib/xor-neon.ko
  AUTOLOAD:=$(call AutoProbe,xor-neon xor)
else
  FILES:=$(LINUX_DIR)/crypto/xor.ko
  AUTOLOAD:=$(call AutoProbe,xor)
endif
endef

define KernelPackage/lib-xor/description
 Kernel module for XOR blocks algorithms
endef

$(eval $(call KernelPackage,lib-xor))


define KernelPackage/lib-textsearch
SUBMENU:=$(LIB_MENU)
  TITLE:=Textsearch support
  KCONFIG:= \
    CONFIG_TEXTSEARCH=y \
    CONFIG_TEXTSEARCH_KMP \
    CONFIG_TEXTSEARCH_BM \
    CONFIG_TEXTSEARCH_FSM
  FILES:= \
    $(LINUX_DIR)/lib/ts_kmp.ko \
    $(LINUX_DIR)/lib/ts_bm.ko \
    $(LINUX_DIR)/lib/ts_fsm.ko
  AUTOLOAD:=$(call AutoProbe,ts_kmp ts_bm ts_fsm)
endef

$(eval $(call KernelPackage,lib-textsearch))


define KernelPackage/lib-zlib-inflate
  SUBMENU:=$(LIB_MENU)
  TITLE:=Zlib support
  HIDDEN:=1
  KCONFIG:=CONFIG_ZLIB_INFLATE
  FILES:=$(LINUX_DIR)/lib/zlib_inflate/zlib_inflate.ko
  AUTOLOAD:=$(call AutoProbe,zlib_inflate)
endef

$(eval $(call KernelPackage,lib-zlib-inflate))


define KernelPackage/lib-zlib-deflate
  SUBMENU:=$(LIB_MENU)
  TITLE:=Zlib support
  HIDDEN:=1
  KCONFIG:=CONFIG_ZLIB_DEFLATE
  FILES:=$(LINUX_DIR)/lib/zlib_deflate/zlib_deflate.ko
  AUTOLOAD:=$(call AutoProbe,zlib_deflate)
endef

$(eval $(call KernelPackage,lib-zlib-deflate))


define KernelPackage/lib-cordic
  SUBMENU:=$(LIB_MENU)
  TITLE:=Cordic function support
  KCONFIG:=CONFIG_CORDIC
  FILES:=$(LINUX_DIR)/lib/math/cordic.ko
  AUTOLOAD:=$(call AutoProbe,cordic)
endef

define KernelPackage/lib-cordic/description
 Kernel module for Cordic function support
endef

$(eval $(call KernelPackage,lib-cordic))


define KernelPackage/asn1-decoder
  SUBMENU:=$(LIB_MENU)
  TITLE:=Simple ASN1 decoder
  KCONFIG:= CONFIG_ASN1
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/lib/asn1_decoder.ko
endef

$(eval $(call KernelPackage,asn1-decoder))

define KernelPackage/asn1-encoder
  SUBMENU:=$(LIB_MENU)
  TITLE:=Simple ASN1 encoder
  KCONFIG:= CONFIG_ASN1_ENCODER
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/lib/asn1_encoder.ko
endef

$(eval $(call KernelPackage,asn1-encoder))

define KernelPackage/oid-registry
  SUBMENU:=$(LIB_MENU)
  TITLE:=Object identifier registry
  KCONFIG:= CONFIG_OID_REGISTRY
  FILES:=$(LINUX_DIR)/lib/oid_registry.ko
  AUTOLOAD:=$(call AutoLoad,31,oid_registry)
endef

$(eval $(call KernelPackage,oid-registry))
