Source-Makefile: feeds/telephony/net/asterisk-16.x/Makefile
Build-Depends: libxml2/host !BUILD_NLS:libiconv !BUILD_NLS:gettext
Build-Types: host

Package: asterisk16
Menu: 1
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread +USE_UCLIBCXX:uclibcxx +USE_LIBCXX:libcxx +USE_LIBSTDCXX:libstdcpp +jansson +libcap +libedit +libopenssl +libsqlite3 +libuuid +libxml2 +zlib
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Complete open source PBX, v16.10.0
Maintainer: Jiri Slachta <jiri@slachta.eu>
Require-User: asterisk=385:asterisk=385
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description:  Asterisk is a complete PBX in software. It provides all of the features
 you would expect from a PBX and more. Asterisk does voice over IP in three
 protocols, and can interoperate with almost all standards-based telephony
 equipment using relatively inexpensive hardware.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@
Config:
	menu "Advanced configuration"
		depends on PACKAGE_asterisk16

	config ASTERISK16_LOW_MEMORY
		bool "Optimize Asterisk 16 for low memory usage"
		default n
		help
		  Warning: this feature is known to cause problems with some modules.
		  Disable it if you experience problems like segmentation faults.

	endmenu
@@

Package: asterisk16-sounds
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Sounds support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides the sound-files for Asterisk 16.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-adsiprog
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-app-adsiprog:asterisk16-res-adsi
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: ADSI programming support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk ADSI programming application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-agent-pool
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Call center agent pool support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Call center agent pool applications.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-alarmreceiver
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Alarm receiver support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Alarm receiver for Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-amd
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Answering machine detection support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Answering Machine Detection application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-attended-transfer
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Attended transfer support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Queues up an attended transfer to a given extension.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-authenticate
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Authenticate commands support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Authentication application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-blind-transfer
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Blind transfer support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Redirects all channels currently bridged to the caller channel to a specified destination.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-bridgeaddchan
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Bridge add channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Bridge-add-channel application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-bridgewait
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-app-bridgewait:asterisk16-bridge-holding
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Holding bridge support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Application to place a channel into a holding bridge.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-celgenuserevent
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: User-defined CEL event support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Generate a user defined CEL event.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-chanisavail
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Channel availability check support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Check channel availability.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-channelredirect
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Redirect a channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Redirects a given channel to a dialplan target.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-chanspy
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Channel listen in support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Listen to the audio of an active channel.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-confbridge
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-app-confbridge:asterisk16-bridge-builtin-features +PACKAGE_asterisk16-app-confbridge:asterisk16-bridge-simple +PACKAGE_asterisk16-app-confbridge:asterisk16-bridge-softmix
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: ConfBridge support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Conference bridge application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-controlplayback
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Control playback support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Control playback application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-dahdiras
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-app-dahdiras:asterisk16-chan-dahdi
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Execute an ISDN RAS support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: DAHDI ISDN Remote Access Server.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-dictate
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Virtual dictation machine support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Virtual dictation machine.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-directed-pickup
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Directed call pickup support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Directed call pickup application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-directory
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Extension directory support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Extension directory.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-disa
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Direct Inward System Access support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Direct Inward System Access application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-dumpchan
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Dump info about channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Dump info about the calling channel.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-exec
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Exec application support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Executes dialplan applications.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-externalivr
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: External IVR interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: External IVR interface application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-festival
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Simple festival interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Simple Festival interface.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-flash
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-app-flash:asterisk16-chan-dahdi
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Flash channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Flash channel application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-followme
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Find-me/follow-me support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Find-Me/Follow-Me application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-getcpeid
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Get ADSI CPE ID support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Get ADSI CPE ID.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-ices
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Encode and stream support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Encode and stream via Icecast and IceS.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-image
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Image transmission support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Image transmission application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-ivrdemo
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IVR demo support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: IVR demo application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-milliwatt
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Digital milliwatt [mu-law] test app support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Digital milliwatt test application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-minivm
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Minimal voicemail system support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: A minimal voicemail e-mail system.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-mixmonitor
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Record a call and mix the audio support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Mixed audio monitoring application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-morsecode
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Morse code support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Morse code.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-mp3
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-app-mp3:mpg123
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Silly MP3 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Silly MP3 application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-originate
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Originate a call support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Originate call.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-page
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-app-page:asterisk16-app-confbridge
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Page multiple phones support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Page multiple phones.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-playtones
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Playtones application support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Playtones application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-privacy
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Require phone number support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Require phone number to be entered if no Caller ID sent.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-queue
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: True Call Queueing support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: True call queueing.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-read
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Variable read support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Read variable application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-readexten
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Extension to variable support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Read and evaluate extension validity.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-record
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Record sound file support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Trivial record application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-saycounted
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Decline words support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Decline words according to channel language.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-sayunixtime
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Say Unix time support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Say time.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-senddtmf
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Send DTMF digits support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Send DTMF digits application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-sendtext
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Send text support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Send text applications.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-skel
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Skeleton [sample] support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Skeleton application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-sms
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SMS support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: SMS/PSTN handler.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-softhangup
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Hang up requested channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Hangs up the requested channel.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-speech
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-app-speech:asterisk16-res-speech
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Dialplan Speech support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Dialplan speech applications.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-stack
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-app-stack:asterisk16-res-agi
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Stack applications support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Dialplan subroutines.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-stasis
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-app-stasis:asterisk16-res-stasis
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Stasis dialplan support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Stasis dialplan application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-statsd
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-app-statsd:asterisk16-res-statsd
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: statsd dialplan support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: StatsD dialplan application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-stream-echo
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Stream echo support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Stream echo application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-system
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: System exec support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Generic system application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-talkdetect
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: File playback with audio detect support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Playback with talk detection.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-test
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Interface test support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Interface test application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-transfer
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Transfers caller to other ext support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Transfers a caller to another extension.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-url
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Send URL support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Send URL applications.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-userevent
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Custom user event support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Custom user event application.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-verbose
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Verbose logging support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Send verbose output.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-waitforring
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Wait for first ring support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Waits until first ring after time.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-waitforsilence
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Wait for silence/noise support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Wait for silence/noise.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-waituntil
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Sleep support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Wait until specified time.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-while
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: While loop support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: While loops and conditional execution.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-app-zapateller
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Block telemarketers support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Block telemarketers with special information tone.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-bridge-builtin-features
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Bridging features support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Built in bridging features.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-bridge-builtin-interval-features
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Built in bridging interval features support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Built in bridging interval features.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-bridge-holding
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Bridging for storing channels in a bridge support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Holding bridge module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-bridge-native-rtp
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Native RTP bridging technology module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Native RTP bridging module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-bridge-simple
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Simple two channel bridging module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Simple two channel bridging module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-bridge-softmix
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Multi-party software based channel mixing support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Multi-party software based channel mixing.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-cdr
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Provides CDR support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Call Detail Records.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-cdr-csv
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Provides CDR CSV support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Comma Separated Values CDR backend.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-cdr-sqlite3
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 libsqlite3
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Provides CDR SQLITE3 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: SQLite3 CDR backend.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-cel-custom
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Customizable CSV CEL backend support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Customizable Comma Separated Values CEL backend.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-cel-manager
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: AMI CEL backend support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk Manager Interface CEL backend.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-cel-sqlite3-custom
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SQLite3 custom CEL support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: SQLite3 custom CEL module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-alsa
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-chan-alsa:alsa-lib
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: ALSA channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: ALSA console channel driver.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-bridge-media
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Bridge media channel driver support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Bridge media channel driver.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-console
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-chan-console:portaudio
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Console channel driver support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Console channel driver.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-dahdi
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-chan-dahdi:dahdi-tools-libtonezone +PACKAGE_asterisk16-chan-dahdi:kmod-dahdi +PACKAGE_asterisk16-chan-dahdi:libpri @!aarch64
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: DAHDI channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: DAHDI telephony.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-iax2
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-chan-iax2:asterisk16-res-timing-timerfd
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IAX2 channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Inter Asterisk eXchange.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-mgcp
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: MGCP support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Media Gateway Control Protocol.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-mobile
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-chan-mobile:bluez-libs
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Bluetooth channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Bluetooth mobile device channel driver.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-motif
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-chan-motif:asterisk16-res-xmpp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Jingle channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Motif Jingle channel driver.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-ooh323
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: H.323 channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Objective Systems H.323 channel.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-oss
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: OSS channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: OSS console channel driver.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-rtp
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RTP media channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RTP media channel.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-sip
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-chan-sip:asterisk16-app-confbridge
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Session Initiation Protocol.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-skinny
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Skinny channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Skinny Client Control Protocol.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-chan-unistim
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Unistim channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: UNISTIM protocol.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-codec-a-mu
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Alaw to ulaw translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Alaw and ulaw direct coder/decoder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-codec-adpcm
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: ADPCM text support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Adaptive Differential PCM coder/decoder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-codec-alaw
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Signed linear to alaw translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Alaw coder/decoder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-codec-dahdi
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-codec-dahdi:asterisk16-chan-dahdi
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: DAHDI codec support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Generic DAHDI transcoder codec translator.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-codec-g722
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: G.722 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: ITU G.722-64kbps G722 transcoder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-codec-g726
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Signed linear to G.726 translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: ITU G.726-32kbps G726 transcoder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-codec-gsm
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: linear to GSM translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: GSM coder/decoder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-codec-ilbc
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: linear to ILBC translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: iLBC coder/decoder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-codec-lpc10
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Linear to LPC10 translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: LPC10 2.4kbps coder/decoder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-codec-resample
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: resample sLinear audio support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: SLIN resampling codec.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-codec-speex
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 @!SOFT_FLOAT +PACKAGE_asterisk16-codec-speex:libspeex +PACKAGE_asterisk16-codec-speex:libspeexdsp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Speex Coder/Decoder support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Speex coder/decoder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-codec-ulaw
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Signed linear to ulaw translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Ulaw coder/decoder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-curl
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-curl:libcurl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: CURL support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: cURL support
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-g719
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: G.719 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: ITU G.719.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-g723
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: G.723.1 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: G.723.1 simple timestamp file format.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-g726
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: G.726 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Raw G.726 data.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-g729
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: G.729 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Raw G.729 data.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-gsm
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: GSM format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Raw GSM data.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-h263
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: H263 format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Raw H.263 data.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-h264
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: H264 format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Raw H.264 data.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-ilbc
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: ILBC format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Raw iLBC data.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-mp3
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 @BROKEN
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: MP3 format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: MP3 format.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-ogg-speex
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 @!SOFT_FLOAT +PACKAGE_asterisk16-format-ogg-speex:libogg +PACKAGE_asterisk16-format-ogg-speex:libspeex
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: OGG/Speex audio support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: OGG/Speex audio.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-ogg-vorbis
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-format-ogg-vorbis:libvorbis
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: OGG/Vorbis audio support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: OGG/Vorbis audio.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-pcm
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: PCM format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Raw/Sun ulaw/alaw 8KHz and G.722 16Khz.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-siren14
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Siren14 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: ITU G.722.1 Annex C.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-siren7
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Siren7 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: ITU G.722.1.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-sln
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Raw slinear format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Raw signed linear audio support 8khz-192khz.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-vox
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: VOX format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Dialogic VOX file format.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-wav
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: WAV format (8000hz Signed Linear) support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Microsoft WAV/WAV16 format.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-format-wav-gsm
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: WAV format (Proprietary GSM) support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Microsoft WAV format.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-aes
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: AES dialplan functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: AES dialplan functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-base64
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: base64 support support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Base64 encode/decode dialplan functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-blacklist
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Blacklist on callerid support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Look up Caller ID name/number from blacklist database.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-callcompletion
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Call control configuration function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Call control configuration function.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-channel
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Channel info support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Channel information dialplan functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-config
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Configuration file variable access support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk configuration file variable access.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-cut
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: CUT function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Cut out information from a string.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-db
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Database interaction support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Database related dialplan functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-devstate
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Blinky lights control support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Gets or sets a device state in the dialplan.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-dialgroup
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Dialgroup dialplan function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Dialgroup dialplan function.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-dialplan
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Dialplan context/extension/priority checking functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Dialplan context/extension/priority checking functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-enum
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: ENUM support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: ENUM related dialplan functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-env
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Environment functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Environment/filesystem dialplan functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-extstate
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Hinted extension state support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Gets the state of an extension in the dialplan.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-frame-trace
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Frame trace for internal ast_frame debugging support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Frame trace for internal ast_frame debugging.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-global
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Global variable support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Variable dialplan functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-groupcount
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Group count support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Channel group dialplan functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-hangupcause
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: HANGUPCAUSE related functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Hangup cause related functions and applications.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-holdintercept
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Hold interception dialplan function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Hold interception dialplan function.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-iconv
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +BUILD_NLS:libiconv-full
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Charset conversion support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Charset conversions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-jitterbuffer
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Jitter buffer for read side of channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Jitter buffer for read side of channel.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-lock
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Dialplan mutexes support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Dialplan mutexes.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-math
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Math functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Mathematical dialplan function.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-md5
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: MD5 digest dialplan functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: MD5 digest dialplan functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-module
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Simple module check function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Checks if Asterisk module is loaded in memory.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-periodic-hook
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-func-periodic-hook:asterisk16-app-chanspy +PACKAGE_asterisk16-func-periodic-hook:asterisk16-func-cut +PACKAGE_asterisk16-func-periodic-hook:asterisk16-func-groupcount +PACKAGE_asterisk16-func-periodic-hook:asterisk16-func-uri
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Periodic dialplan hooks support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Periodic dialplan hooks.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-pitchshift
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Audio effects dialplan functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Audio effects dialplan functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-presencestate
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Hinted presence state support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Gets or sets a presence state in the dialplan.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-rand
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RAND dialplan function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Random number dialplan function.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-realtime
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: REALTIME dialplan function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Read/write/store/destroy values from a realtime repository.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-sha1
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SHA-1 computation dialplan function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: SHA-1 computation dialplan function.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-shell
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Shell support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Collects the output generated by a command executed by the system shell.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-sorcery
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Get a field from a sorcery object support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Get a field from a sorcery object.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-speex
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 @!SOFT_FLOAT +PACKAGE_asterisk16-func-speex:libspeex +PACKAGE_asterisk16-func-speex:libspeexdsp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Noise reduction and AGC support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Noise reduction and Automatic Gain Control.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-sprintf
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SPRINTF dialplan function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: SPRINTF dialplan function.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-srv
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SRV functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: SRV related dialplan functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-sysinfo
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: System information related functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: System information related functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-talkdetect
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Talk detection dialplan function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Talk detection dialplan function.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-uri
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: URI encoding and decoding support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: URI encode/decode dialplan functions.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-version
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Get Asterisk version/build info support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Get Asterisk version/build info.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-vmcount
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: vmcount dialplan support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Indicator for whether a voice mailbox has messages in a given folder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-func-volume
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Technology independent volume control support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Technology independent volume control.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-odbc
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-odbc:libpthread +PACKAGE_asterisk16-odbc:libc +PACKAGE_asterisk16-odbc:unixodbc
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: ODBC support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: ODBC support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-pbx-ael
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-pbx-ael:asterisk16-res-ael-share
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Asterisk Extension Logic support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk Extension Language compiler.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-pbx-dundi
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Dundi support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Distributed Universal Number Discovery.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-pbx-loopback
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Loopback switch support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Loopback switch.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-pbx-lua
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-pbx-lua:liblua
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Lua support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Lua PBX switch.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-pbx-realtime
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Realtime Switch support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Realtime switch.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-pbx-spool
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Call Spool support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Outgoing spool support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-pgsql
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-pgsql:libpq
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: PostgreSQL support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: PostgreSQL support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-pjsip
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-pjsip:asterisk16-res-http-websocket +PACKAGE_asterisk16-pjsip:asterisk16-res-pjproject +PACKAGE_asterisk16-pjsip:asterisk16-res-sorcery +PACKAGE_asterisk16-pjsip:libpjsip +PACKAGE_asterisk16-pjsip:libpjmedia +PACKAGE_asterisk16-pjsip:libpjnath +PACKAGE_asterisk16-pjsip:libpjsip-simple +PACKAGE_asterisk16-pjsip:libpjsip-ua +PACKAGE_asterisk16-pjsip:libpjsua +PACKAGE_asterisk16-pjsip:libpjsua2
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: pjsip channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: PJSIP SIP stack.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-adsi
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Provide ADSI support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: ADSI resource.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ael-share
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Shareable AEL code support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Shareable code for AEL.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-agi
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-agi:asterisk16-res-speech
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Asterisk Gateway Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk Gateway Interface.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-ari:asterisk16-res-http-websocket
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Asterisk RESTful interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk RESTful Interface.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari-applications
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-ari-applications:asterisk16-res-ari +PACKAGE_asterisk16-res-ari-applications:asterisk16-res-ari-model +PACKAGE_asterisk16-res-ari-applications:asterisk16-res-stasis
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RESTful Stasis application resources support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RESTful API module - Stasis application resources.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari-asterisk
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-ari-asterisk:asterisk16-res-ari +PACKAGE_asterisk16-res-ari-asterisk:asterisk16-res-ari-model +PACKAGE_asterisk16-res-ari-asterisk:asterisk16-res-stasis
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RESTful Asterisk resources support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RESTful API module - Asterisk resources.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari-bridges
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-ari-bridges:asterisk16-res-ari +PACKAGE_asterisk16-res-ari-bridges:asterisk16-res-ari-model +PACKAGE_asterisk16-res-ari-bridges:asterisk16-res-stasis-playback
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RESTful bridge resources support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RESTful API module - bridge resources.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari-channels
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-ari-channels:asterisk16-res-ari +PACKAGE_asterisk16-res-ari-channels:asterisk16-res-ari-model +PACKAGE_asterisk16-res-ari-channels:asterisk16-res-stasis-answer +PACKAGE_asterisk16-res-ari-channels:asterisk16-res-stasis-playback +PACKAGE_asterisk16-res-ari-channels:asterisk16-res-stasis-snoop
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RESTful channel resources support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RESTful API module - channel resources.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari-device-states
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-ari-device-states:asterisk16-res-ari +PACKAGE_asterisk16-res-ari-device-states:asterisk16-res-ari-model +PACKAGE_asterisk16-res-ari-device-states:asterisk16-res-stasis-device-state
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RESTful device state resources support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RESTful API module - device state resources.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari-endpoints
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-ari-endpoints:asterisk16-res-ari +PACKAGE_asterisk16-res-ari-endpoints:asterisk16-res-ari-model +PACKAGE_asterisk16-res-ari-endpoints:asterisk16-res-stasis
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RESTful endpoint resources support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RESTful API module - endpoint resources.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari-events
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-ari-events:asterisk16-res-ari +PACKAGE_asterisk16-res-ari-events:asterisk16-res-ari-model +PACKAGE_asterisk16-res-ari-events:asterisk16-res-stasis
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RESTful WebSocket resource support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RESTful API module - WebSocket resource.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari-mailboxes
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-ari-mailboxes:asterisk16-res-ari +PACKAGE_asterisk16-res-ari-mailboxes:asterisk16-res-ari-model +PACKAGE_asterisk16-res-ari-mailboxes:asterisk16-res-stasis-mailbox
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RESTful mailboxes resources support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RESTful API module - mailboxes resources.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari-model
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: ARI model validators support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: ARI model validators.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari-playbacks
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-ari-playbacks:asterisk16-res-ari +PACKAGE_asterisk16-res-ari-playbacks:asterisk16-res-ari-model +PACKAGE_asterisk16-res-ari-playbacks:asterisk16-res-stasis-playback
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RESTful playback control resources support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RESTful API module - playback control resources.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari-recordings
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-ari-recordings:asterisk16-res-ari +PACKAGE_asterisk16-res-ari-recordings:asterisk16-res-ari-model +PACKAGE_asterisk16-res-ari-recordings:asterisk16-res-stasis-recording
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RESTful recording resources support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RESTful API module - recording resources.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-ari-sounds
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-ari-sounds:asterisk16-res-ari +PACKAGE_asterisk16-res-ari-sounds:asterisk16-res-ari-model +PACKAGE_asterisk16-res-ari-sounds:asterisk16-res-stasis
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RESTful sound resources support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RESTful API module - sound resources.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-calendar
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Calendar API support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk calendar integration.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-calendar-caldav
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-calendar-caldav:asterisk16-res-calendar +PACKAGE_asterisk16-res-calendar-caldav:libical +PACKAGE_asterisk16-res-calendar-caldav:libneon
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: CalDAV calendar support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk CalDAV calendar integration.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-calendar-ews
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-calendar-ews:asterisk16-res-calendar +PACKAGE_asterisk16-res-calendar-ews:libneon
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: EWS calendar support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk MS Exchange Web Service calendar integration.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-calendar-exchange
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-calendar-exchange:asterisk16-res-calendar +PACKAGE_asterisk16-res-calendar-exchange:libical +PACKAGE_asterisk16-res-calendar-exchange:libiksemel +PACKAGE_asterisk16-res-calendar-exchange:libneon
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Exchange calendar support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk MS Exchange calendar integration.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-calendar-icalendar
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-calendar-icalendar:asterisk16-res-calendar +PACKAGE_asterisk16-res-calendar-icalendar:libical +PACKAGE_asterisk16-res-calendar-icalendar:libneon
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: iCalendar calendar support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk iCalendar .ics file integration.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-chan-stats
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-chan-stats:asterisk16-res-statsd
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: statsd channel stats support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Example of how to use Stasis.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-clialiases
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: CLI aliases support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: CLI aliases.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-clioriginate
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Calls via CLI support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Call origination and redirection from the CLI.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-config-ldap
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-config-ldap:libopenldap
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: LDAP realtime interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: LDAP realtime interface.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-config-mysql
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-config-mysql:libmysqlclient
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: MySQL CDR backend support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: MySQL realtime configuration driver.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-config-sqlite3
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SQLite 3 realtime config engine support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: SQLite3 realtime config engine.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-convert
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: File format conversion CLI command support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: File format conversion CLI command.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-endpoint-stats
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-endpoint-stats:asterisk16-res-statsd
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Endpoint statistics support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Endpoint statistics.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-hep
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: HEPv3 API support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: HEPv3 API.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-hep-pjsip
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-hep-pjsip:asterisk16-res-hep +PACKAGE_asterisk16-res-hep-pjsip:asterisk16-pjsip
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: PJSIP HEPv3 Logger support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: PJSIP HEPv3 logger.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-hep-rtcp
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-hep-rtcp:asterisk16-res-hep
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RTCP HEPv3 Logger support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: RTCP HEPv3 logger.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-fax-spandsp
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-fax-spandsp:asterisk16-res-fax +PACKAGE_asterisk16-res-fax-spandsp:libspandsp +PACKAGE_asterisk16-res-fax-spandsp:libtiff
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Spandsp T.38 and G.711 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Spandsp G.711 and T.38 FAX technologies.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-fax
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-fax:asterisk16-res-timing-pthread
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: FAX modules support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Generic FAX applications.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-format-attr-celt
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: CELT format attribute module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: CELT format attribute module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-format-attr-g729
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: G.729 format attribute module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: G.729 format attribute module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-format-attr-h263
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: H.263 format attribute module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: H.263 format attribute module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-format-attr-h264
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: H.264 format attribute module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: H.264 format attribute module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-format-attr-ilbc
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: ILBC format attribute module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: iLBC format attribute module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-format-attr-opus
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Opus format attribute module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Opus format attribute module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-format-attr-silk
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SILK format attribute module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: SILK format attribute module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-format-attr-siren14
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Siren14 format attribute module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Siren14 format attribute module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-format-attr-siren7
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Siren7 format attribute module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Siren7 format attribute module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-format-attr-vp8
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: VP8 format attribute module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: VP8 format attribute module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-http-media-cache
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-http-media-cache:asterisk16-curl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: HTTP media cache backend support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: HTTP media cache backend.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-http-websocket
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: HTTP websocket support support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: HTTP WebSocket support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-limit
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Resource limits support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Resource limits.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-manager-devicestate
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Device state topic forwarder support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Manager device state topic forwarder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-manager-presencestate
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Presence state topic forwarder support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Manager presence state topic forwarder.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-monitor
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: PBX channel monitoring support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Call monitoring resource.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-musiconhold
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: MOH support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Music On Hold resource.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-mutestream
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Mute audio stream resources support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Mute audio stream resources.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-mwi-devstate
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: MWI device state subs support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This module allows presence subscriptions to voicemail boxes. This
allows common BLF keys to act as voicemail waiting indicators.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-mwi-external
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Core external MWI resource support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Core external MWI resource.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-mwi-external-ami
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-mwi-external-ami:asterisk16-res-mwi-external
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: AMI for external MWI support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: AMI support for external MWI.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-parking
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-parking:asterisk16-bridge-holding
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Phone Parking support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Call parking resource.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-phoneprov
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Phone Provisioning support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: HTTP phone provisioning.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-pjsip-phoneprov
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-pjsip-phoneprov:asterisk16-pjsip +PACKAGE_asterisk16-res-pjsip-phoneprov:asterisk16-res-phoneprov
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: PJSIP Phone Provisioning support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: PJSIP phone provisioning.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-pjproject
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-pjproject:libpj +PACKAGE_asterisk16-res-pjproject:libpjlib-util +PACKAGE_asterisk16-res-pjproject:libpjmedia +PACKAGE_asterisk16-res-pjproject:libpjmedia +PACKAGE_asterisk16-res-pjproject:libpjnath +PACKAGE_asterisk16-res-pjproject:libpjsip-simple +PACKAGE_asterisk16-res-pjproject:libpjsip-ua +PACKAGE_asterisk16-res-pjproject:libpjsip +PACKAGE_asterisk16-res-pjproject:libpjsua +PACKAGE_asterisk16-res-pjproject:libpjsua2 +PACKAGE_asterisk16-res-pjproject:libsrtp2
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Bridge PJPROJECT to Asterisk logging support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: PJProject log and utility support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-pktccops
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: PktcCOPS manager for MGCP support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: PktcCOPS manager for MGCP.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-realtime
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RealTime CLI support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Realtime data lookup/rewrite.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-remb-modifier
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: REMB modifier support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: REMB modifier module.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-resolver-unbound
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-resolver-unbound:libunbound
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Unbound DNS resolver support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Unbound DNS resolver support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-rtp-asterisk
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-rtp-asterisk:libpjsip +PACKAGE_asterisk16-res-rtp-asterisk:libpjmedia +PACKAGE_asterisk16-res-rtp-asterisk:libpjnath +PACKAGE_asterisk16-res-rtp-asterisk:libpjsip-simple +PACKAGE_asterisk16-res-rtp-asterisk:libpjsip-ua +PACKAGE_asterisk16-res-rtp-asterisk:libpjsua +PACKAGE_asterisk16-res-rtp-asterisk:libpjsua2
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RTP stack support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk RTP stack.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-rtp-multicast
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RTP multicast engine support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Multicast RTP engine.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-security-log
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Security event logging support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Security event logging.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-smdi
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Provide SMDI support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Simplified Message Desk Interface resource.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-snmp
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-snmp:libnetsnmp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SNMP [Sub]Agent for Asterisk support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: SNMP agent for Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-sorcery
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Sorcery data layer support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Sorcery backend modules for data access intended for using realtime as
backend.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-sorcery-memory-cache
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Sorcery memory cache object wizard support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Sorcery memory cache object wizard.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-speech
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Speech Recognition API support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Generic speech recognition API.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-srtp
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-srtp:libsrtp2
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SRTP Support support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Secure RTP.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-stasis
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Stasis application support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Stasis application support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-stasis-answer
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-stasis-answer:asterisk16-res-stasis
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Stasis application answer support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Stasis application answer support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-stasis-device-state
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-stasis-device-state:asterisk16-res-stasis
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Stasis application device state support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Stasis application device state support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-stasis-mailbox
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-stasis-mailbox:asterisk16-res-stasis +PACKAGE_asterisk16-res-stasis-mailbox:asterisk16-res-mwi-external
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Stasis application mailbox support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Stasis application mailbox support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-stasis-playback
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-stasis-playback:asterisk16-res-stasis-recording
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Stasis application playback support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Stasis application playback support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-stasis-recording
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-stasis-recording:asterisk16-res-stasis
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Stasis application recording support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Stasis application recording support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-stasis-snoop
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-stasis-snoop:asterisk16-res-stasis-recording
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Stasis application snoop support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Stasis application snoop support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-statsd
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: statsd client support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Statsd client support.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-stun-monitor
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: STUN monitoring support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: STUN network monitor.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-timing-dahdi
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-timing-dahdi:asterisk16-chan-dahdi
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: DAHDI Timing Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: DAHDI timing interface.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-timing-pthread
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: pthread Timing Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: pthread timing interface.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-timing-timerfd
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Timerfd Timing Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Timerfd timing interface.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-res-xmpp
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-res-xmpp:libiksemel +PACKAGE_asterisk16-res-xmpp:libopenssl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: XMPP client and component module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Asterisk XMPP interface.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-voicemail
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-voicemail:asterisk16-res-adsi +PACKAGE_asterisk16-voicemail:asterisk16-res-smdi
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Voicemail support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Voicemail modules.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-util-aelparse
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-util-aelparse:asterisk16-pbx-ael
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: aelparse utility
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Check extensions.ael file.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-util-astcanary
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: astcanary utility
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Assures Asterisk no threads have gone missing.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-util-astdb2sqlite3
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: astdb2sqlite3 utility
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Convert astdb to SQLite 3.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-util-astdb2bdb
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: astdb2bdb utility
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Convert astdb back to Berkeley DB 1.86.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-util-check-expr
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: check_expr utility
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Expression checker [older version].
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-util-check-expr2
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: check_expr2 utility
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Expression checker [newer version].
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-util-conf2ael
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-util-conf2ael:asterisk16-pbx-ael
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: conf2ael utility
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Convert .conf to .ael.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-util-muted
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: muted utility
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Listens for AMI events. Mutes soundcard during call.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-util-smsq
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16 +PACKAGE_asterisk16-util-smsq:libpopt
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: smsq utility
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Send messages from command line.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-util-stereorize
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: stereorize utility
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: Merge two mono WAV-files to one stereo WAV-file.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk16-util-streamplayer
Submenu: Telephony
Version: 16.10.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk16
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: streamplayer utility
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-16.10.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: A utility for reading from a raw TCP stream [MOH source].
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@


