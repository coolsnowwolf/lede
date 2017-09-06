ifeq ($(BUILD_VARIANT),danube)
  CFLAGS_MODULE = -DCONFIG_DANUBE -DCONFIG_IFXMIPS_DSL_CPE_MEI
  obj-m = ltq_mei_danube.o
  ltq_mei_danube-objs = lantiq_mei.o
endif

ifeq ($(BUILD_VARIANT),ase)
  CFLAGS_MODULE = -DCONFIG_AMAZON_SE -DCONFIG_IFXMIPS_DSL_CPE_MEI
  obj-m = ltq_mei_ase.o
  ltq_mei_ase-objs = lantiq_mei.o
endif

ifeq ($(BUILD_VARIANT),ar9)
  CFLAGS_MODULE = -DCONFIG_AR9 -DCONFIG_IFXMIPS_DSL_CPE_MEI
  obj-m = ltq_mei_ar9.o
  ltq_mei_ar9-objs = lantiq_mei.o
endif
