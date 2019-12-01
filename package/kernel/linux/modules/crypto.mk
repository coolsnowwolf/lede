#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

CRYPTO_MENU:=Cryptographic API modules

CRYPTO_MODULES = \
	ALGAPI2=crypto_algapi \
	BLKCIPHER2=crypto_blkcipher

crypto_confvar=CONFIG_CRYPTO_$(word 1,$(subst =,$(space),$(1)))
crypto_file=$(LINUX_DIR)/crypto/$(word 2,$(subst =,$(space),$(1))).ko
crypto_name=$(if $(findstring y,$($(call crypto_confvar,$(1)))),,$(word 2,$(subst =,$(space),$(1))))

define AddDepends/crypto
  SUBMENU:=$(CRYPTO_MENU)
  DEPENDS+= $(1)
endef


define KernelPackage/crypto-acompress
  TITLE:=Asynchronous Compression operations
  HIDDEN:=1
  KCONFIG:=CONFIG_CRYPTO_ACOMP2
  FILES:=$(LINUX_DIR)/crypto/crypto_acompress.ko
  AUTOLOAD:=$(call AutoLoad,09,crypto_acompress)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-acompress))


define KernelPackage/crypto-aead
  TITLE:=CryptoAPI AEAD support
  KCONFIG:= \
	CONFIG_CRYPTO_AEAD \
	CONFIG_CRYPTO_AEAD2
  FILES:=$(LINUX_DIR)/crypto/aead.ko
  AUTOLOAD:=$(call AutoLoad,09,aead,1)
  $(call AddDepends/crypto, +kmod-crypto-null)
endef

$(eval $(call KernelPackage,crypto-aead))


define KernelPackage/crypto-arc4
  TITLE:=ARC4 cipher CryptoAPI module
  KCONFIG:=CONFIG_CRYPTO_ARC4
  FILES:=$(LINUX_DIR)/crypto/arc4.ko
  AUTOLOAD:=$(call AutoLoad,09,arc4)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-arc4))


define KernelPackage/crypto-authenc
  TITLE:=Combined mode wrapper for IPsec
  DEPENDS:=+kmod-crypto-manager +kmod-crypto-null
  KCONFIG:=CONFIG_CRYPTO_AUTHENC
  FILES:=$(LINUX_DIR)/crypto/authenc.ko
  AUTOLOAD:=$(call AutoLoad,09,authenc)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-authenc))


define KernelPackage/crypto-cbc
  TITLE:=Cipher Block Chaining CryptoAPI module
  DEPENDS:=+kmod-crypto-manager
  KCONFIG:=CONFIG_CRYPTO_CBC
  FILES:=$(LINUX_DIR)/crypto/cbc.ko
  AUTOLOAD:=$(call AutoLoad,09,cbc)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-cbc))


define KernelPackage/crypto-ccm
 TITLE:=Support for Counter with CBC MAC (CCM)
 DEPENDS:=+kmod-crypto-ctr +kmod-crypto-aead
 KCONFIG:=CONFIG_CRYPTO_CCM
 FILES:=$(LINUX_DIR)/crypto/ccm.ko
 AUTOLOAD:=$(call AutoLoad,09,ccm)
 $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-ccm))


define KernelPackage/crypto-cmac
  TITLE:=Support for Cipher-based Message Authentication Code (CMAC)
  DEPENDS:=+kmod-crypto-hash
  KCONFIG:=CONFIG_CRYPTO_CMAC
  FILES:=$(LINUX_DIR)/crypto/cmac.ko
  AUTOLOAD:=$(call AutoLoad,09,cmac)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-cmac))


define KernelPackage/crypto-crc32
  TITLE:=CRC32 CRC module
  DEPENDS:=+kmod-crypto-hash
  KCONFIG:=CONFIG_CRYPTO_CRC32
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/crypto/crc32_generic.ko@ge4.9
  AUTOLOAD:=$(call AutoLoad,04,crc32_generic@ge4.9,1)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-crc32))


define KernelPackage/crypto-crc32c
  TITLE:=CRC32c CRC module
  DEPENDS:=+kmod-crypto-hash
  KCONFIG:=CONFIG_CRYPTO_CRC32C
  FILES:=$(LINUX_DIR)/crypto/crc32c_generic.ko
  AUTOLOAD:=$(call AutoLoad,04,crc32c_generic,1)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-crc32c))


define KernelPackage/crypto-ctr
  TITLE:=Counter Mode CryptoAPI module
  DEPENDS:=+kmod-crypto-manager +kmod-crypto-seqiv +kmod-crypto-iv
  KCONFIG:=CONFIG_CRYPTO_CTR
  FILES:=$(LINUX_DIR)/crypto/ctr.ko
  AUTOLOAD:=$(call AutoLoad,09,ctr)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-ctr))


define KernelPackage/crypto-cts
  TITLE:=Cipher Text Stealing CryptoAPI module
  DEPENDS:=+kmod-crypto-manager
  KCONFIG:=CONFIG_CRYPTO_CTS
  FILES:=$(LINUX_DIR)/crypto/cts.ko
  AUTOLOAD:=$(call AutoLoad,09,cts)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-cts))


define KernelPackage/crypto-deflate
  TITLE:=Deflate compression CryptoAPI module
  DEPENDS:=+kmod-lib-zlib-inflate +kmod-lib-zlib-deflate +!LINUX_4_9:kmod-crypto-acompress
  KCONFIG:=CONFIG_CRYPTO_DEFLATE
  FILES:=$(LINUX_DIR)/crypto/deflate.ko
  AUTOLOAD:=$(call AutoLoad,09,deflate)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-deflate))


define KernelPackage/crypto-des
  TITLE:=DES/3DES cipher CryptoAPI module
  KCONFIG:=CONFIG_CRYPTO_DES
  FILES:=$(LINUX_DIR)/crypto/des_generic.ko
  AUTOLOAD:=$(call AutoLoad,09,des_generic)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-des))


define KernelPackage/crypto-ecb
  TITLE:=Electronic CodeBook CryptoAPI module
  DEPENDS:=+kmod-crypto-manager
  KCONFIG:=CONFIG_CRYPTO_ECB
  FILES:=$(LINUX_DIR)/crypto/ecb.ko
  AUTOLOAD:=$(call AutoLoad,09,ecb)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-ecb))


define KernelPackage/crypto-ecdh
  TITLE:=ECDH algorithm
  DEPENDS:=+kmod-crypto-kpp
  KCONFIG:= CONFIG_CRYPTO_ECDH
  FILES:= \
	$(LINUX_DIR)/crypto/ecdh_generic.ko
  AUTOLOAD:=$(call AutoLoad,10,ecdh_generic)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-ecdh))


define KernelPackage/crypto-echainiv
  TITLE:=Encrypted Chain IV Generator
  DEPENDS:=+kmod-crypto-aead
  KCONFIG:=CONFIG_CRYPTO_ECHAINIV
  FILES:=$(LINUX_DIR)/crypto/echainiv.ko
  AUTOLOAD:=$(call AutoLoad,09,echainiv)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-echainiv))


define KernelPackage/crypto-fcrypt
  TITLE:=FCRYPT cipher CryptoAPI module
  KCONFIG:=CONFIG_CRYPTO_FCRYPT
  FILES:=$(LINUX_DIR)/crypto/fcrypt.ko
  AUTOLOAD:=$(call AutoLoad,09,fcrypt)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-fcrypt))


define KernelPackage/crypto-gcm
  TITLE:=GCM/GMAC CryptoAPI module
  DEPENDS:=+kmod-crypto-ctr +kmod-crypto-ghash +kmod-crypto-null
  KCONFIG:=CONFIG_CRYPTO_GCM
  FILES:=$(LINUX_DIR)/crypto/gcm.ko
  AUTOLOAD:=$(call AutoLoad,09,gcm)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-gcm))


define KernelPackage/crypto-xcbc
  TITLE:=XCBC CryptoAPI module
  DEPENDS:=+kmod-crypto-hash +kmod-crypto-manager
  KCONFIG:=CONFIG_CRYPTO_XCBC
  FILES:=$(LINUX_DIR)/crypto/xcbc.ko
  AUTOLOAD:=$(call AutoLoad,09,xcbc)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-xcbc))


define KernelPackage/crypto-gf128
  TITLE:=GF(2^128) multiplication functions CryptoAPI module
  KCONFIG:=CONFIG_CRYPTO_GF128MUL
  FILES:=$(LINUX_DIR)/crypto/gf128mul.ko
  AUTOLOAD:=$(call AutoLoad,09,gf128mul)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-gf128))


define KernelPackage/crypto-ghash
  TITLE:=GHASH digest CryptoAPI module
  DEPENDS:=+kmod-crypto-gf128 +kmod-crypto-hash
  KCONFIG:=CONFIG_CRYPTO_GHASH
  FILES:=$(LINUX_DIR)/crypto/ghash-generic.ko
  AUTOLOAD:=$(call AutoLoad,09,ghash-generic)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-ghash))


define KernelPackage/crypto-hash
  TITLE:=CryptoAPI hash support
  KCONFIG:=CONFIG_CRYPTO_HASH
  FILES:=$(LINUX_DIR)/crypto/crypto_hash.ko
  AUTOLOAD:=$(call AutoLoad,02,crypto_hash,1)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-hash))


define KernelPackage/crypto-hmac
  TITLE:=HMAC digest CryptoAPI module
  DEPENDS:=+kmod-crypto-hash +kmod-crypto-manager
  KCONFIG:=CONFIG_CRYPTO_HMAC
  FILES:=$(LINUX_DIR)/crypto/hmac.ko
  AUTOLOAD:=$(call AutoLoad,09,hmac)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-hmac))


define KernelPackage/crypto-hw-ccp
  TITLE:=AMD Cryptographic Coprocessor
  DEPENDS:=+kmod-crypto-authenc +kmod-crypto-hash +kmod-crypto-manager +kmod-random-core +kmod-crypto-sha1 +kmod-crypto-sha256 +!LINUX_4_9:kmod-crypto-rsa
  KCONFIG:= \
	CONFIG_CRYPTO_HW=y \
	CONFIG_CRYPTO_DEV_CCP=y \
	CONFIG_CRYPTO_DEV_CCP_CRYPTO \
	CONFIG_CRYPTO_DEV_CCP_DD \
	CONFIG_CRYPTO_DEV_SP_CCP=y
  FILES:= \
	$(LINUX_DIR)/drivers/crypto/ccp/ccp.ko \
	$(LINUX_DIR)/drivers/crypto/ccp/ccp-crypto.ko
  AUTOLOAD:=$(call AutoLoad,09,ccp ccp-crypto)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-hw-ccp))


define KernelPackage/crypto-hw-geode
  TITLE:=AMD Geode hardware crypto module
  DEPENDS:=+kmod-crypto-manager
  KCONFIG:= \
	CONFIG_CRYPTO_HW=y \
	CONFIG_CRYPTO_DEV_GEODE
  FILES:=$(LINUX_DIR)/drivers/crypto/geode-aes.ko
  AUTOLOAD:=$(call AutoLoad,09,geode-aes)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-hw-geode))


define KernelPackage/crypto-hw-hifn-795x
  TITLE:=HIFN 795x crypto accelerator
  DEPENDS:=+kmod-random-core +kmod-crypto-manager
  KCONFIG:= \
	CONFIG_CRYPTO_HW=y \
	CONFIG_CRYPTO_DEV_HIFN_795X \
	CONFIG_CRYPTO_DEV_HIFN_795X_RNG=y
  FILES:=$(LINUX_DIR)/drivers/crypto/hifn_795x.ko
  AUTOLOAD:=$(call AutoLoad,09,hifn_795x)
  $(call AddDepends/crypto,+kmod-crypto-des)
endef

$(eval $(call KernelPackage,crypto-hw-hifn-795x))


define KernelPackage/crypto-hw-padlock
  TITLE:=VIA PadLock ACE with AES/SHA hw crypto module
  DEPENDS:=+kmod-crypto-manager
  KCONFIG:= \
	CONFIG_CRYPTO_HW=y \
	CONFIG_CRYPTO_DEV_PADLOCK \
	CONFIG_CRYPTO_DEV_PADLOCK_AES \
	CONFIG_CRYPTO_DEV_PADLOCK_SHA
  FILES:= \
	$(LINUX_DIR)/drivers/crypto/padlock-aes.ko \
	$(LINUX_DIR)/drivers/crypto/padlock-sha.ko
  AUTOLOAD:=$(call AutoLoad,09,padlock-aes padlock-sha)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-hw-padlock))


define KernelPackage/crypto-hw-safexcel
  TITLE:= MVEBU SafeXcel Crypto Engine module
  DEPENDS:=@!LINUX_4_14 @(TARGET_mvebu_cortexa53||TARGET_mvebu_cortexa72) \
	+kmod-crypto-authenc +kmod-crypto-md5 +kmod-crypto-hmac +kmod-crypto-sha256 +kmod-crypto-sha512
  KCONFIG:= \
	CONFIG_CRYPTO_HW=y \
	CONFIG_CRYPTO_DEV_SAFEXCEL
  FILES:=$(LINUX_DIR)/drivers/crypto/inside-secure/crypto_safexcel.ko
  AUTOLOAD:=$(call AutoLoad,90,crypto_safexcel)
  $(call AddDepends/crypto)
endef

define KernelPackage/crypto-hw-safexcel/description
MVEBU's EIP97 and EIP197 Cryptographic Engine driver designed by
Inside Secure. This is found on Marvell Armada 37xx/7k/8k SoCs.

Particular version of these IP (EIP197B and EIP197D) require firmware.
Unfortunately it's not freely available and needs signed Non-Disclosure
Agreement (NDA) with Marvell. For those who have signed NDA the firmware can be
obtained at https://extranet.marvell.com.
endef

$(eval $(call KernelPackage,crypto-hw-safexcel))


define KernelPackage/crypto-hw-talitos
  TITLE:=Freescale integrated security engine (SEC) driver
  DEPENDS:=+kmod-crypto-manager +kmod-crypto-hash +kmod-random-core +kmod-crypto-authenc +kmod-crypto-des
  KCONFIG:= \
	CONFIG_CRYPTO_HW=y \
	CONFIG_CRYPTO_DEV_TALITOS \
	CONFIG_CRYPTO_DEV_TALITOS1=y \
	CONFIG_CRYPTO_DEV_TALITOS2=y
  FILES:= \
	$(LINUX_DIR)/drivers/crypto/talitos.ko
  AUTOLOAD:=$(call AutoLoad,09,talitos)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-hw-talitos))


define KernelPackage/crypto-iv
  TITLE:=CryptoAPI initialization vectors
  DEPENDS:=+kmod-crypto-manager +kmod-crypto-rng +kmod-crypto-wq
  KCONFIG:= CONFIG_CRYPTO_BLKCIPHER2
  HIDDEN:=1
  FILES:= \
	$(LINUX_DIR)/crypto/eseqiv.ko@lt4.9 \
	$(LINUX_DIR)/crypto/chainiv.ko@lt4.9
  AUTOLOAD:=$(call AutoLoad,10,eseqiv chainiv)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-iv))


define KernelPackage/crypto-kpp
  TITLE:=Key-agreement Protocol Primitives
  KCONFIG:=CONFIG_CRYPTO_KPP
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/crypto/kpp.ko
  AUTOLOAD:=$(call AutoLoad,09,kpp)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-kpp))


define KernelPackage/crypto-manager
  TITLE:=CryptoAPI algorithm manager
  DEPENDS:=+kmod-crypto-aead +kmod-crypto-hash +kmod-crypto-pcompress
  KCONFIG:= \
	CONFIG_CRYPTO_MANAGER \
	CONFIG_CRYPTO_MANAGER2
  FILES:=$(LINUX_DIR)/crypto/cryptomgr.ko
  AUTOLOAD:=$(call AutoLoad,09,cryptomgr,1)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-manager))


define KernelPackage/crypto-md4
  TITLE:=MD4 digest CryptoAPI module
  DEPENDS:=+kmod-crypto-hash
  KCONFIG:=CONFIG_CRYPTO_MD4
  FILES:=$(LINUX_DIR)/crypto/md4.ko
  AUTOLOAD:=$(call AutoLoad,09,md4)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-md4))


define KernelPackage/crypto-md5
  TITLE:=MD5 digest CryptoAPI module
  DEPENDS:=+kmod-crypto-hash
  KCONFIG:= \
	CONFIG_CRYPTO_MD5 \
	CONFIG_CRYPTO_MD5_OCTEON
  FILES:=$(LINUX_DIR)/crypto/md5.ko
  AUTOLOAD:=$(call AutoLoad,09,md5)
  $(call AddDepends/crypto)
endef

define KernelPackage/crypto-md5/octeon
  FILES+=$(LINUX_DIR)/arch/mips/cavium-octeon/crypto/octeon-md5.ko
  AUTOLOAD+=$(call AutoLoad,09,octeon-md5)
endef

$(eval $(call KernelPackage,crypto-md5))


define KernelPackage/crypto-michael-mic
  TITLE:=Michael MIC keyed digest CryptoAPI module
  DEPENDS:=+kmod-crypto-hash
  KCONFIG:=CONFIG_CRYPTO_MICHAEL_MIC
  FILES:=$(LINUX_DIR)/crypto/michael_mic.ko
  AUTOLOAD:=$(call AutoLoad,09,michael_mic)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-michael-mic))


define KernelPackage/crypto-misc
  TITLE:=Other CryptoAPI modules
  DEPENDS:=+kmod-crypto-xts
  KCONFIG:= \
	CONFIG_CRYPTO_CAMELLIA_X86_64 \
	CONFIG_CRYPTO_BLOWFISH_X86_64 \
	CONFIG_CRYPTO_TWOFISH_X86_64 \
	CONFIG_CRYPTO_TWOFISH_X86_64_3WAY \
	CONFIG_CRYPTO_SERPENT_SSE2_X86_64 \
	CONFIG_CRYPTO_CAMELLIA_AESNI_AVX_X86_64 \
	CONFIG_CRYPTO_CAST5_AVX_X86_64 \
	CONFIG_CRYPTO_CAST6_AVX_X86_64 \
	CONFIG_CRYPTO_TWOFISH_AVX_X86_64 \
	CONFIG_CRYPTO_SERPENT_AVX_X86_64 \
	CONFIG_CRYPTO_CAMELLIA_AESNI_AVX2_X86_64 \
	CONFIG_CRYPTO_SERPENT_AVX2_X86_64 \
	CONFIG_CRYPTO_SERPENT_SSE2_586 \
	CONFIG_CRYPTO_ANUBIS \
	CONFIG_CRYPTO_BLOWFISH \
	CONFIG_CRYPTO_CAMELLIA \
	CONFIG_CRYPTO_CAST5 \
	CONFIG_CRYPTO_CAST6 \
	CONFIG_CRYPTO_FCRYPT \
	CONFIG_CRYPTO_KHAZAD \
	CONFIG_CRYPTO_SERPENT \
	CONFIG_CRYPTO_TEA \
	CONFIG_CRYPTO_TGR192 \
	CONFIG_CRYPTO_TWOFISH \
	CONFIG_CRYPTO_TWOFISH_COMMON \
	CONFIG_CRYPTO_TWOFISH_586 \
	CONFIG_CRYPTO_WP512
  FILES:= \
	$(LINUX_DIR)/crypto/anubis.ko \
	$(LINUX_DIR)/crypto/camellia_generic.ko \
	$(LINUX_DIR)/crypto/cast_common.ko \
	$(LINUX_DIR)/crypto/cast5_generic.ko \
	$(LINUX_DIR)/crypto/cast6_generic.ko \
	$(LINUX_DIR)/crypto/khazad.ko \
	$(LINUX_DIR)/crypto/tea.ko \
	$(LINUX_DIR)/crypto/tgr192.ko \
	$(LINUX_DIR)/crypto/twofish_common.ko \
	$(LINUX_DIR)/crypto/wp512.ko \
	$(LINUX_DIR)/crypto/twofish_generic.ko \
	$(LINUX_DIR)/crypto/blowfish_common.ko \
	$(LINUX_DIR)/crypto/blowfish_generic.ko \
	$(LINUX_DIR)/crypto/serpent_generic.ko
  AUTOLOAD:=$(call AutoLoad,10,anubis camellia_generic cast_common \
	cast5_generic cast6_generic khazad tea tgr192 twofish_common \
	wp512 blowfish_common serpent_generic)
  ifndef CONFIG_TARGET_x86
	AUTOLOAD+= $(call AutoLoad,10,twofish_generic blowfish_generic)
  endif
  $(call AddDepends/crypto)
endef

ifndef CONFIG_TARGET_x86_64
  define KernelPackage/crypto-misc/x86
    FILES+= \
	$(LINUX_DIR)/arch/x86/crypto/twofish-i586.ko \
	$(LINUX_DIR)/arch/x86/crypto/serpent-sse2-i586.ko \
	$(LINUX_DIR)/arch/x86/crypto/glue_helper.ko \
	$(LINUX_DIR)/crypto/ablk_helper.ko@lt4.17 \
	$(LINUX_DIR)/crypto/cryptd.ko \
	$(LINUX_DIR)/crypto/lrw.ko@lt4.17 \
	$(LINUX_DIR)/crypto/crypto_simd.ko@ge4.17
    AUTOLOAD+= $(call AutoLoad,10,cryptd glue_helper \
	serpent-sse2-i586 twofish-i586 blowfish_generic)
  endef
endif

define KernelPackage/crypto-misc/x86/64
  FILES+= \
	$(LINUX_DIR)/arch/x86/crypto/camellia-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/blowfish-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/twofish-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/twofish-x86_64-3way.ko \
	$(LINUX_DIR)/arch/x86/crypto/serpent-sse2-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/camellia-aesni-avx-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/cast5-avx-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/cast6-avx-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/twofish-avx-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/serpent-avx-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/camellia-aesni-avx2.ko \
	$(LINUX_DIR)/arch/x86/crypto/serpent-avx2.ko \
	$(LINUX_DIR)/crypto/ablk_helper.ko@lt4.17
  AUTOLOAD+= $(call AutoLoad,10,camellia-x86_64 \
	camellia-aesni-avx-x86_64 camellia-aesni-avx2 cast5-avx-x86_64 \
	cast6-avx-x86_64 twofish-x86_64 twofish-x86_64-3way \
	twofish-avx-x86_64 blowfish-x86_64 serpent-avx-x86_64 serpent-avx2)
endef

$(eval $(call KernelPackage,crypto-misc))


define KernelPackage/crypto-null
  TITLE:=Null CryptoAPI module
  KCONFIG:=CONFIG_CRYPTO_NULL
  FILES:=$(LINUX_DIR)/crypto/crypto_null.ko
  AUTOLOAD:=$(call AutoLoad,09,crypto_null)
  $(call AddDepends/crypto, +kmod-crypto-hash)
endef

$(eval $(call KernelPackage,crypto-null))


define KernelPackage/crypto-pcbc
  TITLE:=Propagating Cipher Block Chaining CryptoAPI module
  DEPENDS:=+kmod-crypto-manager
  KCONFIG:=CONFIG_CRYPTO_PCBC
  FILES:=$(LINUX_DIR)/crypto/pcbc.ko
  AUTOLOAD:=$(call AutoLoad,09,pcbc)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-pcbc))


define KernelPackage/crypto-pcompress
  TITLE:=CryptoAPI Partial (de)compression operations
  KCONFIG:= \
	CONFIG_CRYPTO_PCOMP=y \
	CONFIG_CRYPTO_PCOMP2
  FILES:=$(LINUX_DIR)/crypto/pcompress.ko
  AUTOLOAD:=$(call AutoLoad,09,pcompress)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-pcompress))


define KernelPackage/crypto-rsa
  TITLE:=RSA algorithm
  DEPENDS:=+kmod-crypto-manager +kmod-asn1-decoder
  KCONFIG:= CONFIG_CRYPTO_RSA
  HIDDEN:=1
  FILES:= \
	$(LINUX_DIR)/lib/mpi/mpi.ko \
	$(LINUX_DIR)/crypto/akcipher.ko \
	$(LINUX_DIR)/crypto/rsa_generic.ko
  AUTOLOAD:=$(call AutoLoad,10,rsa_generic)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-rsa))


define KernelPackage/crypto-rmd160
  TITLE:=RIPEMD160 digest CryptoAPI module
  DEPENDS:=+kmod-crypto-hash
  KCONFIG:=CONFIG_CRYPTO_RMD160
  FILES:=$(LINUX_DIR)/crypto/rmd160.ko
  AUTOLOAD:=$(call AutoLoad,09,rmd160)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-rmd160))


define KernelPackage/crypto-rng
  TITLE:=CryptoAPI random number generation
  DEPENDS:=+kmod-crypto-hash +kmod-crypto-hmac +kmod-crypto-sha256
  KCONFIG:= \
	CONFIG_CRYPTO_DRBG \
	CONFIG_CRYPTO_DRBG_HMAC=y \
	CONFIG_CRYPTO_DRBG_HASH=n \
	CONFIG_CRYPTO_DRBG_MENU \
	CONFIG_CRYPTO_JITTERENTROPY \
	CONFIG_CRYPTO_RNG2
  FILES:= \
	$(LINUX_DIR)/crypto/drbg.ko@ge4.2 \
	$(LINUX_DIR)/crypto/jitterentropy_rng.ko@ge4.2 \
	$(LINUX_DIR)/crypto/krng.ko@lt4.2 \
	$(LINUX_DIR)/crypto/rng.ko
  AUTOLOAD:=$(call AutoLoad,09,drbg@ge4.2 jitterentropy_rng@ge4.2 krng@lt4.2 rng)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-rng))


define KernelPackage/crypto-seqiv
  TITLE:=CryptoAPI Sequence Number IV Generator
  DEPENDS:=+kmod-crypto-aead +kmod-crypto-rng
  KCONFIG:=CONFIG_CRYPTO_SEQIV
  FILES:=$(LINUX_DIR)/crypto/seqiv.ko
  AUTOLOAD:=$(call AutoLoad,09,seqiv)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-seqiv))


define KernelPackage/crypto-sha1
  TITLE:=SHA1 digest CryptoAPI module
  DEPENDS:=+kmod-crypto-hash
  KCONFIG:= \
	CONFIG_CRYPTO_SHA1 \
	CONFIG_CRYPTO_SHA1_OCTEON \
	CONFIG_CRYPTO_SHA1_SSSE3
  FILES:=$(LINUX_DIR)/crypto/sha1_generic.ko
  AUTOLOAD:=$(call AutoLoad,09,sha1_generic)
  $(call AddDepends/crypto)
endef

define KernelPackage/crypto-sha1/octeon
  FILES+=$(LINUX_DIR)/arch/mips/cavium-octeon/crypto/octeon-sha1.ko
  AUTOLOAD+=$(call AutoLoad,09,octeon-sha1)
endef

define KernelPackage/crypto-sha1/x86/64
  FILES+=$(LINUX_DIR)/arch/x86/crypto/sha1-ssse3.ko
  AUTOLOAD+=$(call AutoLoad,09,sha1-ssse3)
endef

$(eval $(call KernelPackage,crypto-sha1))


define KernelPackage/crypto-sha256
  TITLE:=SHA224 SHA256 digest CryptoAPI module
  DEPENDS:=+kmod-crypto-hash
  KCONFIG:= \
	CONFIG_CRYPTO_SHA256 \
	CONFIG_CRYPTO_SHA256_OCTEON \
	CONFIG_CRYPTO_SHA256_SSSE3
  FILES:=$(LINUX_DIR)/crypto/sha256_generic.ko
  AUTOLOAD:=$(call AutoLoad,09,sha256_generic)
  $(call AddDepends/crypto)
endef

define KernelPackage/crypto-sha256/octeon
  FILES+=$(LINUX_DIR)/arch/mips/cavium-octeon/crypto/octeon-sha256.ko
  AUTOLOAD+=$(call AutoLoad,09,octeon-sha256)
endef

define KernelPackage/crypto-sha256/x86/64
  FILES+=$(LINUX_DIR)/arch/x86/crypto/sha256-ssse3.ko
  AUTOLOAD+=$(call AutoLoad,09,sha256-ssse3)
endef

$(eval $(call KernelPackage,crypto-sha256))


define KernelPackage/crypto-sha512
  TITLE:=SHA512 digest CryptoAPI module
  DEPENDS:=+kmod-crypto-hash
  KCONFIG:= \
	CONFIG_CRYPTO_SHA512 \
	CONFIG_CRYPTO_SHA512_OCTEON \
	CONFIG_CRYPTO_SHA512_SSSE3
  FILES:=$(LINUX_DIR)/crypto/sha512_generic.ko
  AUTOLOAD:=$(call AutoLoad,09,sha512_generic)
  $(call AddDepends/crypto)
endef

define KernelPackage/crypto-sha512/octeon
  FILES+=$(LINUX_DIR)/arch/mips/cavium-octeon/crypto/octeon-sha512.ko
  AUTOLOAD+=$(call AutoLoad,09,octeon-sha512)
endef

define KernelPackage/crypto-sha512/x86/64
  FILES+=$(LINUX_DIR)/arch/x86/crypto/sha512-ssse3.ko
  AUTOLOAD+=$(call AutoLoad,09,sha512-ssse3)
endef

$(eval $(call KernelPackage,crypto-sha512))


define KernelPackage/crypto-test
  TITLE:=Test CryptoAPI module
  KCONFIG:=CONFIG_CRYPTO_TEST
  FILES:=$(LINUX_DIR)/crypto/tcrypt.ko
  $(call AddDepends/crypto,+kmod-crypto-manager)
endef

$(eval $(call KernelPackage,crypto-test))


define KernelPackage/crypto-user
  TITLE:=CryptoAPI userspace interface
  DEPENDS:=+kmod-crypto-hash +kmod-crypto-manager
  KCONFIG:= \
	CONFIG_CRYPTO_USER_API \
	CONFIG_CRYPTO_USER_API_AEAD \
	CONFIG_CRYPTO_USER_API_HASH \
	CONFIG_CRYPTO_USER_API_RNG \
	CONFIG_CRYPTO_USER_API_SKCIPHER
  FILES:= \
	$(LINUX_DIR)/crypto/af_alg.ko \
	$(LINUX_DIR)/crypto/algif_aead.ko \
	$(LINUX_DIR)/crypto/algif_hash.ko \
	$(LINUX_DIR)/crypto/algif_rng.ko \
	$(LINUX_DIR)/crypto/algif_skcipher.ko
  AUTOLOAD:=$(call AutoLoad,09,af_alg algif_aead algif_hash algif_rng algif_skcipher)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-user))


define KernelPackage/crypto-wq
  TITLE:=CryptoAPI work queue handling
  KCONFIG:=CONFIG_CRYPTO_WORKQUEUE
  FILES:=$(LINUX_DIR)/crypto/crypto_wq.ko
  AUTOLOAD:=$(call AutoLoad,09,crypto_wq)
  $(call AddDepends/crypto)
endef
$(eval $(call KernelPackage,crypto-wq))


define KernelPackage/crypto-xts
  TITLE:=XTS cipher CryptoAPI module
  DEPENDS:=+kmod-crypto-gf128 +kmod-crypto-manager
  KCONFIG:=CONFIG_CRYPTO_XTS
  FILES:=$(LINUX_DIR)/crypto/xts.ko
  AUTOLOAD:=$(call AutoLoad,09,xts)
  $(call AddDepends/crypto)
endef

$(eval $(call KernelPackage,crypto-xts))

