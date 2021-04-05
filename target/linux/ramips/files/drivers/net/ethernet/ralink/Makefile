#
# Makefile for the Ralink SoCs built-in ethernet macs
#

ralink-eth-y					+= mtk_eth_soc.o ethtool.o

ralink-eth-$(CONFIG_NET_RALINK_MDIO)		+= mdio.o
ralink-eth-$(CONFIG_NET_RALINK_MDIO_RT2880)	+= mdio_rt2880.o
ralink-eth-$(CONFIG_NET_RALINK_MDIO_MT7620)	+= mdio_mt7620.o

ralink-eth-$(CONFIG_NET_RALINK_OFFLOAD)	+= mtk_offload.o mtk_debugfs.o

ralink-eth-$(CONFIG_NET_RALINK_RT2880)	+= soc_rt2880.o
ralink-eth-$(CONFIG_NET_RALINK_RT3050)	+= soc_rt3050.o
ralink-eth-$(CONFIG_NET_RALINK_RT3883)	+= soc_rt3883.o
ralink-eth-$(CONFIG_NET_RALINK_MT7620)	+= soc_mt7620.o
ralink-eth-$(CONFIG_NET_RALINK_MT7621)	+= soc_mt7621.o

obj-$(CONFIG_NET_RALINK_ESW_RT3050)		+= esw_rt3050.o
obj-$(CONFIG_NET_RALINK_GSW_MT7620)		+= gsw_mt7620.o mt7530.o
obj-$(CONFIG_NET_RALINK_GSW_MT7621)		+= gsw_mt7621.o mt7530.o
obj-$(CONFIG_NET_RALINK_SOC)			+= ralink-eth.o
