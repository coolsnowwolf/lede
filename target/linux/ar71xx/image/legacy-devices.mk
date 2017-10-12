define LegacyDevice/A60
   DEVICE_TITLE := OpenMesh A40/A60
   DEVICE_PACKAGES := om-watchdog kmod-ath10k ath10k-firmware-qca988x \
	mod-usb-core kmod-usb2
endef
LEGACY_DEVICES += A60

define LegacyDevice/ALFANX
  DEVICE_TITLE := ALFA Network N2/N5 board
endef
LEGACY_DEVICES += ALFANX

define LegacyDevice/HORNETUB
  DEVICE_TITLE := ALFA Network Hornet-UB board (8MB flash, 32MB ram)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += HORNETUB

define LegacyDevice/TUBE2H8M
  DEVICE_TITLE := ALFA Network Tube2H board (8MB flash)
endef
LEGACY_DEVICES += TUBE2H8M

define LegacyDevice/AP96
  DEVICE_TITLE := Atheros AP96 reference board
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
endef
LEGACY_DEVICES += AP96

define LegacyDevice/WNDAP360
  DEVICE_TITLE := NETGEAR WNDAP360
endef
LEGACY_DEVICES += WNDAP360

define LegacyDevice/ALFAAP120C
  DEVICE_TITLE := ALFA Network AP120C board
endef
LEGACY_DEVICES += ALFAAP120C

define LegacyDevice/ALFAAP96
  DEVICE_TITLE := ALFA Network AP96 board
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-rtc-pcf2123
endef
LEGACY_DEVICES += ALFAAP96

define LegacyDevice/ALL0258N
  DEVICE_TITLE := Allnet ALL0258N
  DEVICE_PACKAGES := rssileds
endef
LEGACY_DEVICES += ALL0258N

define LegacyDevice/ALL0315N
  DEVICE_TITLE := Allnet ALL0315N
  DEVICE_PACKAGES := rssileds
endef
LEGACY_DEVICES += ALL0315N

define LegacyDevice/AP121_8M
  DEVICE_TITLE := Atheros AP121 reference board (8MB flash)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
endef
LEGACY_DEVICES += AP121_8M

define LegacyDevice/AP121_16M
  DEVICE_TITLE := Atheros AP121 reference board (16MB flash)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
endef
LEGACY_DEVICES += AP121_16M

define LegacyDevice/AP132
  DEVICE_TITLE := Atheros AP132 reference board
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage
endef
LEGACY_DEVICES += AP132

define LegacyDevice/AP135
  DEVICE_TITLE := Atheros AP135 reference board
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage
endef
LEGACY_DEVICES += AP135

define LegacyDevice/AP136_010
  DEVICE_TITLE := Atheros AP136-010 reference board
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage
endef
LEGACY_DEVICES += AP136_010

define LegacyDevice/AP136_020
  DEVICE_TITLE := Atheros AP136-020 reference board
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage
endef
LEGACY_DEVICES += AP136_020

define LegacyDevice/AP143_8M
  DEVICE_TITLE := Qualcomm Atheros AP143 reference board (8MB flash)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage
endef
LEGACY_DEVICES += AP143_8M

define LegacyDevice/AP143_16M
  DEVICE_TITLE := Qualcomm Atheros AP143 reference board (16MB flash)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage
endef
LEGACY_DEVICES += AP143_16M

define LegacyDevice/AP147_010
  DEVICE_TITLE := Qualcomm Atheros AP147-010 reference board
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage
endef
LEGACY_DEVICES += AP147_010

define LegacyDevice/AP152_16M
  DEVICE_TITLE := Qualcomm Atheros AP152 reference board (16MB flash)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage
endef
LEGACY_DEVICES += AP152_16M

define LegacyDevice/BXU2000N2
  DEVICE_TITLE := BHU BXU2000n-2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage
endef
LEGACY_DEVICES += BXU2000N2

define LegacyDevice/CAP4200AG
  DEVICE_TITLE := Senao CAP4200AG
endef
LEGACY_DEVICES += CAP4200AG

define LegacyDevice/DB120
  DEVICE_TITLE := Atheros DB120 reference board
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage
endef
LEGACY_DEVICES += DB120

define LegacyDevice/HORNETUBx2
  DEVICE_TITLE := ALFA Network Hornet-UB-x2 board (16MB flash, 64MB ram)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += HORNETUBx2

define LegacyDevice/TUBE2H16M
  DEVICE_TITLE := ALFA Network Tube2H board (16MB flash)
endef
LEGACY_DEVICES += TUBE2H16M

define LegacyDevice/WLR8100
  DEVICE_TITLE := Sitecom WLR-8100
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-usb3
endef
LEGACY_DEVICES += WLR8100

define LegacyDevice/F9K1115V2
  DEVICE_TITLE := Belkin AC1750DB (F9K1115V2)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb3 kmod-usb-ledtrig-usbport \
	kmod-ath10k ath10k-firmware-qca988x
endef
LEGACY_DEVICES += F9K1115V2

define LegacyDevice/DIR600A1
  DEVICE_TITLE := D-Link DIR-600 rev. A1
endef
LEGACY_DEVICES += DIR600A1

define LegacyDevice/DIR601A1
  DEVICE_TITLE := D-Link DIR-601 rev. A1
endef
LEGACY_DEVICES += DIR601A1

define LegacyDevice/FR54RTR
  DEVICE_TITLE := Frys FR-54RTR
endef
LEGACY_DEVICES += FR54RTR

define LegacyDevice/EBR2310C1
  DEVICE_TITLE := D-Link EBR-2310 rev. C1
endef
LEGACY_DEVICES += EBR2310C1

define LegacyDevice/DIR615E1
  DEVICE_TITLE := D-Link DIR-615 rev. E1
endef
LEGACY_DEVICES += DIR615E1

define LegacyDevice/DIR615E4
  DEVICE_TITLE := D-Link DIR-615 rev. E4
endef
LEGACY_DEVICES += DIR615E4

define LegacyDevice/DIR615I1
  DEVICE_TITLE := D-Link DIR-615 rev. I1
endef
LEGACY_DEVICES += DIR615I1

define LegacyDevice/DIR615I3
  DEVICE_TITLE := D-Link DIR-615 rev. I3
endef
LEGACY_DEVICES += DIR615I3

define LegacyDevice/A02RBW300N
  DEVICE_TITLE := Atlantis-Land A02-RB-W300N
endef
LEGACY_DEVICES += A02RBW300N

define LegacyDevice/DIR615C1
  DEVICE_TITLE := D-Link DIR-615 rev. C1
endef
LEGACY_DEVICES += DIR615C1

define LegacyDevice/TEW632BRP
  DEVICE_TITLE := TRENDNet TEW-632BRP
endef
LEGACY_DEVICES += TEW632BRP

define LegacyDevice/TEW652BRP_FW
  DEVICE_TITLE := TRENDNet TEW-652BRP
endef
LEGACY_DEVICES += TEW652BRP_FW

define LegacyDevice/TEW652BRP_RECOVERY
  DEVICE_TITLE := TRENDNet TEW-652BRP (recovery)
endef
LEGACY_DEVICES += TEW652BRP_RECOVERY

define LegacyDevice/TEW712BR
  DEVICE_TITLE := TRENDNet TEW-712BR
endef
LEGACY_DEVICES += TEW712BR

define LegacyDevice/DIR601B1
  DEVICE_TITLE := D-Link DIR-601 rev. B1
endef
LEGACY_DEVICES += DIR601B1

define LegacyDevice/DIR505A1
  DEVICE_TITLE := D-Link DIR-505 rev. A1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += DIR505A1

define LegacyDevice/DGL5500A1
  DEVICE_TITLE := D-Link DGL-5500 rev. A1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ath10k ath10k-firmware-qca988x
endef
LEGACY_DEVICES += DGL5500A1

define LegacyDevice/TEW823DRU
  DEVICE_TITLE := TRENDNet TEW-823DRU
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ath10k ath10k-firmware-qca988x
endef
LEGACY_DEVICES += TEW823DRU

define LegacyDevice/DHP1565A1
  DEVICE_TITLE := D-Link DHP-1565 rev. A1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
endef
LEGACY_DEVICES += DHP1565A1

define LegacyDevice/DIR825C1
  DEVICE_TITLE := D-Link DIR-825 rev. C1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += DIR825C1

define LegacyDevice/DIR835A1
  DEVICE_TITLE := D-Link DIR-835 rev. A1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
endef
LEGACY_DEVICES += DIR835A1

define LegacyDevice/TEW732BR
  DEVICE_TITLE := TRENDNet TEW-732BR
endef
LEGACY_DEVICES += TEW732BR

define LegacyDevice/WRT160NL
  DEVICE_TITLE := Linksys WRT160NL
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
endef
LEGACY_DEVICES += WRT160NL

define LegacyDevice/MYNETREXT
  DEVICE_TITLE := Western Digital My Net Wi-Fi Range Extender
  DEVICE_PACKAGES := rssileds
endef
LEGACY_DEVICES += MYNETREXT

define LegacyDevice/DIR825B1
  DEVICE_TITLE := D-Link DIR-825 rev. B1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += DIR825B1

define LegacyDevice/TEW673GRU
  DEVICE_TITLE := TRENDNet TEW-673GRU
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2
endef
LEGACY_DEVICES += TEW673GRU

define LegacyDevice/DLRTDEV01
  DEVICE_TITLE := PowerCloud Systems dlrtdev01 model
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += DLRTDEV01

define LegacyDevice/dLAN_Hotspot
  DEVICE_TITLE := devolo dLAN Hotspot
endef
LEGACY_DEVICES += dLAN_Hotspot

define LegacyDevice/dLAN_pro_500_wp
  DEVICE_TITLE := devolo dLAN pro 500 Wireless+
endef
LEGACY_DEVICES += dLAN_pro_500_wp

define LegacyDevice/dLAN_pro_1200_ac
  DEVICE_TITLE := devolo dLAN pro 1200+ WiFi ac
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
endef
LEGACY_DEVICES += dLAN_pro_1200_ac

define LegacyDevice/ESR900
  DEVICE_TITLE := EnGenius ESR900
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += ESR900

define LegacyDevice/ESR1750
  DEVICE_TITLE := EnGenius ESR1750
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage kmod-ath10k ath10k-firmware-qca988x
endef
LEGACY_DEVICES += ESR1750

define LegacyDevice/EPG5000
  DEVICE_TITLE := EnGenius EPG5000
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage kmod-ath10k ath10k-firmware-qca988x
endef
LEGACY_DEVICES += EPG5000

define LegacyDevice/WP543_4M
  DEVICE_TITLE := Compex WP543/WPJ543 (4MB flash)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2
endef
LEGACY_DEVICES += WP543_4M

define LegacyDevice/WP543_8M
  DEVICE_TITLE := Compex WP543/WPJ543 (8MB flash)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2
endef
LEGACY_DEVICES += WP543_8M

define LegacyDevice/WP543_16M
  DEVICE_TITLE := Compex WP543/WPJ543 (16MB flash)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2
endef
LEGACY_DEVICES += WP543_16M

define LegacyDevice/WPE72_4M
  DEVICE_TITLE := Compex WPE72/WPE72NX (4MB flash)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2
endef
LEGACY_DEVICES += WPE72_4M

define LegacyDevice/WPE72_8M
  DEVICE_TITLE := Compex WPE72/WPE72NX (8MB flash)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2
endef
LEGACY_DEVICES += WPE72_8M

define LegacyDevice/WPE72_16M
  DEVICE_TITLE := Compex WPE72/WPE72NX (16MB flash)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2
endef
LEGACY_DEVICES += WPE72_16M

define LegacyDevice/WNR2000
  DEVICE_TITLE := NETGEAR WNR2000V1
endef
LEGACY_DEVICES += WNR2000

define LegacyDevice/WNR2000V3
  DEVICE_TITLE := NETGEAR WNR2000V3
endef
LEGACY_DEVICES += WNR2000V3

define LegacyDevice/WNR2000V4
  DEVICE_TITLE := NETGEAR WNR2000V4
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += WNR2000V4

define LegacyDevice/WNR2200
  DEVICE_TITLE := NETGEAR WNR2200
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += WNR2200

define LegacyDevice/REALWNR612V2
  DEVICE_TITLE := NETGEAR WNR612V2
endef
LEGACY_DEVICES += REALWNR612V2

define LegacyDevice/N150R
  DEVICE_TITLE := On Networks N150
endef
LEGACY_DEVICES += N150R

define LegacyDevice/REALWNR1000V2
  DEVICE_TITLE := NETGEAR WNR1000V2
endef
LEGACY_DEVICES += REALWNR1000V2

define LegacyDevice/WNR1000V2_VC
  DEVICE_TITLE := NETGEAR WNR1000V2-VC
endef
LEGACY_DEVICES += WNR1000V2_VC

define LegacyDevice/WPN824N
  DEVICE_TITLE := NETGEAR WPN824N
endef
LEGACY_DEVICES += WPN824N

define LegacyDevice/OM2P
  DEVICE_TITLE := OpenMesh OM2P/OM2P-HS/OM2P-LC
  DEVICE_PACKAGES := om-watchdog
endef
LEGACY_DEVICES += OM2P

define LegacyDevice/OM5P
  DEVICE_TITLE := OpenMesh OM5P/OM5P-AN
  DEVICE_PACKAGES := om-watchdog
endef
LEGACY_DEVICES += OM5P

define LegacyDevice/OM5PAC
  DEVICE_TITLE := OpenMesh OM5P-AC
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x om-watchdog
endef
LEGACY_DEVICES += OM5PAC

define LegacyDevice/MR600
  DEVICE_TITLE := OpenMesh MR600
  DEVICE_PACKAGES := om-watchdog
endef
LEGACY_DEVICES += MR600

define LegacyDevice/MR900
  DEVICE_TITLE := OpenMesh MR900
  DEVICE_PACKAGES := om-watchdog
endef
LEGACY_DEVICES += MR900

define LegacyDevice/MR1750
  DEVICE_TITLE := OpenMesh MR1750
  DEVICE_PACKAGES := om-watchdog kmod-ath10k ath10k-firmware-qca988x
endef
LEGACY_DEVICES += MR1750

define LegacyDevice/ALL0305
  DEVICE_TITLE := Allnet ALL0305
  DEVICE_PACKAGES := fconfig kmod-ath5k -kmod-ath9k
endef
LEGACY_DEVICES += ALL0305

define LegacyDevice/EAP7660D
  DEVICE_TITLE := Senao EAP7660D
endef
LEGACY_DEVICES += EAP7660D

define LegacyDevice/JA76PF
  DEVICE_TITLE := jjPlus JA76PF
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-hwmon-core kmod-i2c-core kmod-hwmon-lm75
endef
LEGACY_DEVICES += JA76PF

define LegacyDevice/JA76PF2
  DEVICE_TITLE := jjPlus JA76PF2
endef
LEGACY_DEVICES += JA76PF2

define LegacyDevice/JWAP003
  DEVICE_TITLE := jjPlus JWAP003
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2
endef
LEGACY_DEVICES += JWAP003

define LegacyDevice/PB42
  DEVICE_TITLE := Atheros PB42 reference board
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2
endef
LEGACY_DEVICES += PB42

define LegacyDevice/PB44
  DEVICE_TITLE := Atheros PB44 reference board
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 \
	  vsc7385-ucode-pb44 vsc7395-ucode-pb44
endef
LEGACY_DEVICES += PB44

define LegacyDevice/MZKW04NU
  DEVICE_TITLE := Planex MZK-W04NU
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += MZKW04NU

define LegacyDevice/MZKW300NH
  DEVICE_TITLE := Planex MZK-W300NH
endef
LEGACY_DEVICES += MZKW300NH

define LegacyDevice/EAP300V2
  DEVICE_TITLE := EnGenius EAP300V2
endef
LEGACY_DEVICES += EAP300V2

define LegacyDevice/WHRG301N
  DEVICE_TITLE := Buffalo WHR-G301N
endef
LEGACY_DEVICES += WHRG301N

define LegacyDevice/WHRHPG300N
  DEVICE_TITLE := Buffalo WHR-HP-G300N
endef
LEGACY_DEVICES += WHRHPG300N

define LegacyDevice/WHRHPGN
  DEVICE_TITLE := Buffalo WHR-HP-GN
endef
LEGACY_DEVICES += WHRHPGN

define LegacyDevice/WLAEAG300N
  DEVICE_TITLE := Buffalo WLAE-AG300N
  DEVICE_PACKAGES := kmod-ledtrig-netdev
endef
LEGACY_DEVICES += WLAEAG300N

define LegacyDevice/WRT400N
  DEVICE_TITLE := Linksys WRT400N
endef
LEGACY_DEVICES += WRT400N

define LegacyDevice/WZRHPG300NH
  DEVICE_TITLE := Buffalo WZR-HP-G300NH
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += WZRHPG300NH

define LegacyDevice/WZRHPG300NH2
  DEVICE_TITLE := Buffalo WZR-HP-G300NH2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += WZRHPG300NH2

define LegacyDevice/WZRHPAG300H
  DEVICE_TITLE := Buffalo WZR-HP-AG300H
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += WZRHPAG300H

define LegacyDevice/WZRHPG450H
  DEVICE_TITLE := Buffalo WZR-HP-G450H
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += WZRHPG450H

define LegacyDevice/WZR600DHP
  DEVICE_TITLE := Buffalo WZR-600DHP
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += WZR600DHP

define LegacyDevice/WZR450HP2
  DEVICE_TITLE := Buffalo WZR-450HP2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += WZR450HP2

define LegacyDevice/ZCN1523H28
  DEVICE_TITLE := Zcomax ZCN-1523H-2-8
endef
LEGACY_DEVICES += ZCN1523H28

define LegacyDevice/ZCN1523H516
  DEVICE_TITLE := Zcomax ZCN-1523H-5-16
endef
LEGACY_DEVICES += ZCN1523H516

define LegacyDevice/NBG_460N_550N_550NH
  DEVICE_TITLE := ZyXEL NBG 460N/550N/550NH
  DEVICE_PACKAGES := kmod-rtc-pcf8563
endef
LEGACY_DEVICES += NBG_460N_550N_550NH
