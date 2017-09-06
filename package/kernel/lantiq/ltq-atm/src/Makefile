ifeq ($(BUILD_VARIANT),danube)
  CFLAGS_MODULE = -DCONFIG_DANUBE
  obj-m = ltq_atm_danube.o
  ltq_atm_danube-objs = ltq_atm.o ifxmips_atm_danube.o
endif

ifeq ($(BUILD_VARIANT),ase)
  CFLAGS_MODULE = -DCONFIG_AMAZON_SE
  obj-m = ltq_atm_ase.o
  ltq_atm_ase-objs = ltq_atm.o ifxmips_atm_amazon_se.o
endif

ifeq ($(BUILD_VARIANT),ar9)
  CFLAGS_MODULE = -DCONFIG_AR9
  obj-m = ltq_atm_ar9.o
  ltq_atm_ar9-objs = ltq_atm.o ifxmips_atm_ar9.o
endif

ifeq ($(BUILD_VARIANT),vr9)
  CFLAGS_MODULE = -DCONFIG_VR9
  obj-m = ltq_atm_vr9.o
  ltq_atm_vr9-objs = ltq_atm.o ifxmips_atm_vr9.o
endif
