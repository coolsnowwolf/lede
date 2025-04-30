#
# Makefile for the Atheros AR71xx built-in ethernet macs
#

ag71xx_legacy-y	+= ag71xx_main.o
ag71xx_legacy-y	+= ag71xx_gmac.o
ag71xx_legacy-y	+= ag71xx_ethtool.o
ag71xx_legacy-y	+= ag71xx_phy.o

ag71xx_legacy-$(CONFIG_AG71XX_LEGACY_DEBUG_FS)	+= ag71xx_legacy_debugfs.o

obj-$(CONFIG_AG71XX_LEGACY)	+= ag71xx_legacy_mdio.o
obj-$(CONFIG_AG71XX_LEGACY)	+= ag71xx_legacy.o
