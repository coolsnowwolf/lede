ifeq ($(BUILD_VARIANT),danube)
  CFLAGS_MODULE =-DCONFIG_DANUBE -DCONFIG_CRYPTO_DEV_DEU -DCONFIG_CRYPTO_DEV_SPEED_TEST -DCONFIG_CRYPTO_DEV_DES \
  		-DCONFIG_CRYPTO_DEV_AES -DCONFIG_CRYPTO_DEV_SHA1 -DCONFIG_CRYPTO_DEV_MD5
  obj-m = ltq_deu_danube.o
  ltq_deu_danube-objs = ifxmips_deu.o ifxmips_deu_danube.o ifxmips_des.o ifxmips_aes.o ifxmips_sha1.o ifxmips_md5.o
endif

ifeq ($(BUILD_VARIANT),ar9)
  CFLAGS_MODULE = -DCONFIG_AR9 -DCONFIG_CRYPTO_DEV_DEU -DCONFIG_CRYPTO_DEV_SPEED_TEST -DCONFIG_CRYPTO_DEV_DES \
  		-DCONFIG_CRYPTO_DEV_AES -DCONFIG_CRYPTO_DEV_SHA1 -DCONFIG_CRYPTO_DEV_MD5 -DCONFIG_CRYPTO_DEV_ARC4 \
		-DCONFIG_CRYPTO_DEV_SHA1_HMAC -DCONFIG_CRYPTO_DEV_MD5_HMAC
  obj-m = ltq_deu_ar9.o
  ltq_deu_ar9-objs = ifxmips_deu.o ifxmips_deu_ar9.o ifxmips_des.o ifxmips_aes.o ifxmips_arc4.o \
  			ifxmips_sha1.o ifxmips_md5.o ifxmips_sha1_hmac.o ifxmips_md5_hmac.o
endif

ifeq ($(BUILD_VARIANT),vr9)
  CFLAGS_MODULE = -DCONFIG_VR9 -DCONFIG_CRYPTO_DEV_DEU -DCONFIG_CRYPTO_DEV_SPEED_TEST -DCONFIG_CRYPTO_DEV_DES \
  		-DCONFIG_CRYPTO_DEV_AES -DCONFIG_CRYPTO_DEV_SHA1 -DCONFIG_CRYPTO_DEV_MD5 -DCONFIG_CRYPTO_DEV_ARC4 \
		-DCONFIG_CRYPTO_DEV_SHA1_HMAC -DCONFIG_CRYPTO_DEV_MD5_HMAC
  obj-m = ltq_deu_vr9.o
  ltq_deu_vr9-objs = ifxmips_deu.o ifxmips_deu_vr9.o ifxmips_des.o ifxmips_aes.o ifxmips_arc4.o \
  			ifxmips_sha1.o ifxmips_md5.o ifxmips_sha1_hmac.o ifxmips_md5_hmac.o
endif
