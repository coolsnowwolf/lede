ccflags-y += $(AIROHA_PON_CFLAGS)

obj-m += airoha_ecnt_hook.o
airoha_ecnt_hook-y := core/ecnt_hook.o module/ecnt_hook_meta.o

obj-m += airoha_ecnt_scu.o
airoha_ecnt_scu-y := core/ecnt_scu.o module/ecnt_scu_meta.o

obj-m += airoha_ecnt_pon_phy.o
airoha_ecnt_pon_phy-y := core/ecnt_pon_phy.o core/legacy_api.o module/ecnt_pon_phy_meta.o

obj-m += airoha_ecnt_olt_phy.o
airoha_ecnt_olt_phy-y := core/ecnt_olt_phy.o module/ecnt_olt_phy_meta.o

obj-m += airoha_arht_board.o
airoha_arht_board-y := core/arht_board.o module/arht_board_meta.o

obj-m += airoha_arht_serdes.o
airoha_arht_serdes-y := core/arht_serdes_cfg.o module/arht_serdes_meta.o

obj-m += airoha_an7583_combo_phy.o
airoha_an7583_combo_phy-y := \
	core/ecnt_efuse.o \
	an7583/ecnt_serdes_common_phy.o \
	an7583/xfi_pma/src/xfi_pma.o \
	an7583/pon_pma/src/pon_pma.o \
	an7583/pcie0_pma/src/pcie0_pma.o \
	module/an7583_combo_phy_meta.o
