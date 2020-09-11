Source-Makefile: feeds/telephony/net/kamailio-5.x/Makefile
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:gettext

Package: kamailio5
Menu: 1
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread +BUILD_NLS:libiconv-full +libncurses +libpthread +libreadline +libxml2
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Mature and flexible open source SIP server, v5.4.0
Maintainer: Jiri Slachta <jiri@slachta.eu>
Require-User: kamailio=380:kamailio=380
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Mature and flexible open source SIP server, v5.4.0
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-lib-libkamailio-ims
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Kamailio5 IMS library
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Kamailio5 IMS library
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-lib-libtrie
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Kamailio5 digital tree library
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Kamailio5 digital tree library
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-util-kambdb-recover
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-util-kambdb-recover:kamailio5-mod-db-berkeley
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Kamailio5 Berkeley DB recovery utility
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Kamailio5 Berkeley DB recovery utility
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-acc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-acc:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Accounting for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Accounting for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-acc-diameter
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-acc-diameter:kamailio5-mod-acc
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Accounting for DIAMETER backend for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Accounting for DIAMETER backend for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-acc-json
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-acc-json:jansson +PACKAGE_kamailio5-mod-acc-json:kamailio5-mod-acc
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Accounting with records exported in JSON format for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Accounting with records exported in JSON format for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-alias-db
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Database-backend aliases for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Database-backend aliases for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-app-jsdt
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Execute JavaScript scripts for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Execute JavaScript scripts for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-app-lua
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-app-lua:liblua
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Execute embedded Lua scripts for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Execute embedded Lua scripts for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-app-lua-sr
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-app-lua-sr:kamailio5-mod-app-lua
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Old Lua API for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Old Lua API for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-app-python3
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-app-python3:python3-light
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Python3 scripting interpreter for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Python3 scripting interpreter for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-app-ruby
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-app-ruby:libruby
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Ruby scripting interpreter for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Ruby scripting interpreter for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-app-sqlang
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-app-sqlang:libstdcpp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Execute Squirrel language scripts for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Execute Squirrel language scripts for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-async
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-async:kamailio5-mod-tm +PACKAGE_kamailio5-mod-async:kamailio5-mod-tmx
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Asynchronous SIP handling functions for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Asynchronous SIP handling functions for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-auth
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Authentication Framework for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Authentication Framework for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-auth-db
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-auth-db:kamailio5-mod-auth
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Database-backend authentication for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Database-backend authentication for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-auth-diameter
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-auth-diameter:kamailio5-mod-sl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Diameter authentication for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Diameter authentication for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-auth-ephemeral
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-auth-ephemeral:libopenssl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Ephemeral credentials for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Ephemeral credentials for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-auth-identity
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-auth-identity:libopenssl +PACKAGE_kamailio5-mod-auth-identity:libcurl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Identity authentication for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Identity authentication for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-auth-xkeys
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-auth-xkeys:kamailio5-mod-auth
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Shared-key authentication for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Shared-key authentication for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-avp
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Functions for handling AVPs for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Functions for handling AVPs for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-avpops
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: AVP operation for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: AVP operation for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-benchmark
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Config benchmark for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Config benchmark for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-blst
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Blacklisting API for config for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Blacklisting API for config for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-call-control
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-call-control:kamailio5-mod-dialog +PACKAGE_kamailio5-mod-call-control:kamailio5-mod-pv
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Call Control for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Call Control for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-call-obj
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Call identification support for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Call identification support for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-carrierroute
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-carrierroute:kamailio5-lib-libtrie
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Carrier Route for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Carrier Route for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-cdp
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: C Diameter Peer for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: C Diameter Peer for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-cdp-avp
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-cdp-avp:kamailio5-mod-cdp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: CDP AVP helper module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: CDP AVP helper module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-cfgutils
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Config utilities for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Config utilities for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-cfg-db
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Load parameters from database for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Load parameters from database for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-cfg-rpc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Update parameters via RPC for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Update parameters via RPC for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-cfgt
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Unit test reporting for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Unit test reporting for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-cnxcc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-cnxcc:kamailio5-mod-dialog +PACKAGE_kamailio5-mod-cnxcc:libhiredis +PACKAGE_kamailio5-mod-cnxcc:libevent2
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Limit call duration for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Limit call duration for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-corex
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Legacy functions for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Legacy functions for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-counters
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Functions for counter manipulation for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Functions for counter manipulation for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-cplc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-cplc:kamailio5-mod-sl +PACKAGE_kamailio5-mod-cplc:kamailio5-mod-tm +PACKAGE_kamailio5-mod-cplc:kamailio5-mod-usrloc
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Call Processing Language interpreter for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Call Processing Language interpreter for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-crypto
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-crypto:libopenssl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Various cryptography tools for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Various cryptography tools for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ctl
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: BINRPC transport interface for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: BINRPC transport interface for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-db2-ldap
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-db2-ldap:libopenldap
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: DBv2 LDAP module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: DBv2 LDAP module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-db2-ops
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Run SQL queries from script for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Run SQL queries from script for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-db-berkeley
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-db-berkeley:libdb47
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Berkeley DB backend for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Berkeley DB backend for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-db-cluster
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Database clustering system for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Database clustering system for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-db-flatstore
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: fast write-only text DB-backend for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: fast write-only text DB-backend for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-db-mysql
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-db-mysql:libmysqlclient
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: MySQL DB-backend for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: MySQL DB-backend for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-db-postgres
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-db-postgres:libpq
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: PostgreSQL DB-backend for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: PostgreSQL DB-backend for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-db-redis
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-db-redis:libhiredis
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Database backend with Redis server for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Database backend with Redis server for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-db-sqlite
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-db-sqlite:libsqlite3
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SQLite DB-backend for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SQLite DB-backend for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-db-text
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Text DB-backend for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Text DB-backend for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-db-unixodbc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-db-unixodbc:unixodbc
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: UnixODBC DB-backend for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: UnixODBC DB-backend for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-debugger
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Interactive config file debugger for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Interactive config file debugger for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-dialog
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-dialog:kamailio5-mod-rr +PACKAGE_kamailio5-mod-dialog:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Dialog support for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Dialog support for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-dialplan
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-dialplan:libpcre
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Dialplan management for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Dialplan management for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-dispatcher
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Dispatcher for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Dispatcher for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-diversion
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Diversion header insertion for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Diversion header insertion for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-dlgs
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Track active calls in stateless mode for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Track active calls in stateless mode for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-dmq
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-dmq:kamailio5-mod-sl +PACKAGE_kamailio5-mod-dmq:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Distributed Message Queue for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Distributed Message Queue for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-dmq-usrloc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-dmq-usrloc:kamailio5-mod-dmq +PACKAGE_kamailio5-mod-dmq-usrloc:kamailio5-mod-usrloc
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: DMQ USRLOC replication for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: DMQ USRLOC replication for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-domain
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Multi-domain support for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Multi-domain support for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-domainpolicy
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Domain policy for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Domain policy for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-drouting
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Dynamic routing module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Dynamic routing module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-enum
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: ENUM lookup for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: ENUM lookup for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-erlang
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 @!USE_MUSL +PACKAGE_kamailio5-mod-erlang:erlang
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Erlang node connector module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Erlang node connector module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-evapi
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-evapi:libev
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: push event details via tcp for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: push event details via tcp for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-evrexec
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Execut event routes at startup for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Execut event routes at startup for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-exec
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: External exec for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: External exec for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-group
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Database-backend user-groups for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Database-backend user-groups for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-gzcompress
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-gzcompress:zlib
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Compress SIP messages for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Compress SIP messages for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-h350
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-h350:kamailio5-mod-ldap +PACKAGE_kamailio5-mod-h350:libopenldap
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: H.350 for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: H.350 for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-htable
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Hash Table for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Hash Table for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-http-client
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-http-client:libcurl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: HTTP client using CURL for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: HTTP client using CURL for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-imc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-imc:kamailio5-mod-db-mysql +PACKAGE_kamailio5-mod-imc:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IM conferencing for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: IM conferencing for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-auth
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ims-auth:kamailio5-lib-libkamailio-ims +PACKAGE_kamailio5-mod-ims-auth:kamailio5-mod-cdp +PACKAGE_kamailio5-mod-ims-auth:kamailio5-mod-cdp-avp +PACKAGE_kamailio5-mod-ims-auth:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IMS authentication module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: IMS authentication module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-charging
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ims-charging:kamailio5-lib-libkamailio-ims +PACKAGE_kamailio5-mod-ims-charging:kamailio5-mod-cdp +PACKAGE_kamailio5-mod-ims-charging:kamailio5-mod-cdp-avp +PACKAGE_kamailio5-mod-ims-charging:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IMS charging component module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: IMS charging component module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-dialog
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ims-dialog:kamailio5-mod-rr +PACKAGE_kamailio5-mod-ims-dialog:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IMS dialog tracking module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: IMS dialog tracking module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-diameter-server
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ims-diameter-server:kamailio5-lib-libkamailio-ims +PACKAGE_kamailio5-mod-ims-diameter-server:kamailio5-mod-cdp +PACKAGE_kamailio5-mod-ims-diameter-server:kamailio5-mod-cdp-avp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IMS DIAMETER server module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: IMS DIAMETER server module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-icscf
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ims-icscf:kamailio5-lib-libkamailio-ims +PACKAGE_kamailio5-mod-ims-icscf:kamailio5-mod-cdp +PACKAGE_kamailio5-mod-ims-icscf:kamailio5-mod-cdp-avp +PACKAGE_kamailio5-mod-ims-icscf:kamailio5-mod-sl +PACKAGE_kamailio5-mod-ims-icscf:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IMS ICSCF component module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: IMS ICSCF component module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-ipsec-pcscf
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ims-ipsec-pcscf:kamailio5-lib-libkamailio-ims +PACKAGE_kamailio5-mod-ims-ipsec-pcscf:kamailio5-mod-ims-usrloc-pcscf +PACKAGE_kamailio5-mod-ims-ipsec-pcscf:kamailio5-mod-tm +PACKAGE_kamailio5-mod-ims-ipsec-pcscf:libmnl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Diameter server implementation for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Diameter server implementation for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-isc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ims-isc:kamailio5-lib-libkamailio-ims +PACKAGE_kamailio5-mod-ims-isc:kamailio5-mod-ims-usrloc-scscf +PACKAGE_kamailio5-mod-ims-isc:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IMS ISC component module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: IMS ISC component module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-ocs
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ims-ocs:kamailio5-lib-libkamailio-ims +PACKAGE_kamailio5-mod-ims-ocs:kamailio5-mod-cdp +PACKAGE_kamailio5-mod-ims-ocs:kamailio5-mod-cdp-avp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: MS OCS component module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: MS OCS component module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-qos
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ims-qos:kamailio5-lib-libkamailio-ims +PACKAGE_kamailio5-mod-ims-qos:kamailio5-mod-cdp +PACKAGE_kamailio5-mod-ims-qos:kamailio5-mod-cdp-avp +PACKAGE_kamailio5-mod-ims-qos:kamailio5-mod-ims-dialog +PACKAGE_kamailio5-mod-ims-qos:kamailio5-mod-ims-usrloc-pcscf +PACKAGE_kamailio5-mod-ims-qos:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IMS Diameter Rx interface between PCSCF and PCRF functions for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: IMS Diameter Rx interface between PCSCF and PCRF functions for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-registrar-pcscf
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ims-registrar-pcscf:kamailio5-lib-libkamailio-ims +PACKAGE_kamailio5-mod-ims-registrar-pcscf:kamailio5-mod-ims-usrloc-pcscf
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: MS PCSCF registrar module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: MS PCSCF registrar module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-registrar-scscf
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ims-registrar-scscf:kamailio5-lib-libkamailio-ims +PACKAGE_kamailio5-mod-ims-registrar-scscf:kamailio5-mod-cdp +PACKAGE_kamailio5-mod-ims-registrar-scscf:kamailio5-mod-cdp-avp +PACKAGE_kamailio5-mod-ims-registrar-scscf:kamailio5-mod-ims-usrloc-scscf +PACKAGE_kamailio5-mod-ims-registrar-scscf:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IMS SCSCF registrar module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: IMS SCSCF registrar module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-usrloc-pcscf
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IMS PCSCF usrloc module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: IMS PCSCF usrloc module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ims-usrloc-scscf
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IMS SCSCF usrloc module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: IMS SCSCF usrloc module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ipops
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: IP and IPv6 operations for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: IP and IPv6 operations for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-jansson
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-jansson:jansson
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Access to JSON attributes for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Access to JSON attributes for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-janssonrpcc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-janssonrpcc:kamailio5-mod-jansson +PACKAGE_kamailio5-mod-janssonrpcc:libevent2
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Alternative JSONRPC server for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Alternative JSONRPC server for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-json
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-json:libjson-c
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Access to JSON document attributes for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Access to JSON document attributes for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-jsonrpcs
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-jsonrpcs:libevent2
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: JSONRPC server over HTTP for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: JSONRPC server over HTTP for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-keepalive
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP keepalive monitoring for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP keepalive monitoring for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-kemix
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: KEMI extensions for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: KEMI extensions for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-kex
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Core extensions for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Core extensions for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-lcr
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-lcr:kamailio5-mod-tm +PACKAGE_kamailio5-mod-lcr:libpcre
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Least Cost Routing for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Least Cost Routing for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ldap
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ldap:libopenldap
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: LDAP connector for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: LDAP connector for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-log-custom
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Logging to custom backends for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Logging to custom backends for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-lost
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-lost:kamailio5-mod-http-client
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: HELD and LOST routing for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: HELD and LOST routing for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-mangler
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SDP mangling for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SDP mangling for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-matrix
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Matrix operations for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Matrix operations for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-maxfwd
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Max-Forward processor for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Max-Forward processor for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-mediaproxy
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-mediaproxy:kamailio5-mod-dialog
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Automatic NAT traversal for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Automatic NAT traversal for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-mohqueue
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-mohqueue:kamailio5-mod-rtpproxy +PACKAGE_kamailio5-mod-mohqueue:kamailio5-mod-sl +PACKAGE_kamailio5-mod-mohqueue:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Music on hold queuing system for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Music on hold queuing system for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-mqueue
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Generic message queue system for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Generic message queue system for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-msilo
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-msilo:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP message silo for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP message silo for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-msrp
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-msrp:kamailio5-mod-tls
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: MSRP routing engine for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: MSRP routing engine for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-mtree
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Memory caching system for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Memory caching system for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-nathelper
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-nathelper:kamailio5-mod-usrloc
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: NAT helper for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: NAT helper for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-nat-traversal
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-nat-traversal:kamailio5-mod-dialog +PACKAGE_kamailio5-mod-nat-traversal:kamailio5-mod-sl +PACKAGE_kamailio5-mod-nat-traversal:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: NAT traversal for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: NAT traversal for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ndb-redis
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-ndb-redis:libhiredis
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Connect to REDIS NoSQL for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Connect to REDIS NoSQL for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-nosip
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-nosip:kamailio5-mod-rr
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: non-sip package handling for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: non-sip package handling for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-outbound
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-outbound:kamailio5-mod-stun +PACKAGE_kamailio5-mod-outbound:libopenssl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP Outbound implementation for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP Outbound implementation for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-p-usrloc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Partitioned USRLOC services for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Partitioned USRLOC services for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-path
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-path:kamailio5-mod-rr
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP path insertion for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP path insertion for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pdb
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Number portability module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Number portability module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pdt
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Prefix-to-Domain translator for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Prefix-to-Domain translator for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-permissions
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Permissions control for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Permissions control for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pike
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Flood detector for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Flood detector for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pipelimit
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-pipelimit:kamailio5-mod-sl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Traffic shaping policies for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Traffic shaping policies for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-prefix-route
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Execute based on prefix for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Execute based on prefix for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-presence
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-presence:kamailio5-mod-sl +PACKAGE_kamailio5-mod-presence:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Presence server for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Presence server for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-presence-conference
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-presence-conference:kamailio5-mod-presence
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Conference events for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Conference events for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-presence-dialoginfo
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-presence-dialoginfo:kamailio5-mod-presence
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Dialog Event presence for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Dialog Event presence for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-presence-mwi
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-presence-mwi:kamailio5-mod-presence
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: MWI presence for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: MWI presence for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-presence-profile
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-presence-profile:kamailio5-mod-presence
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: User profile extensions for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: User profile extensions for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-presence-reginfo
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-presence-reginfo:kamailio5-mod-presence
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Registration info for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Registration info for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-presence-xml
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-presence-xml:kamailio5-mod-presence +PACKAGE_kamailio5-mod-presence-xml:kamailio5-mod-xcap-client
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: XCAP presence for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: XCAP presence for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pua
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-pua:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Presence User Agent for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Presence User Agent for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pua-bla
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-pua-bla:kamailio5-mod-presence +PACKAGE_kamailio5-mod-pua-bla:kamailio5-mod-pua +PACKAGE_kamailio5-mod-pua-bla:kamailio5-mod-usrloc
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Bridged Line Appearence PUA for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Bridged Line Appearence PUA for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pua-dialoginfo
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-pua-dialoginfo:kamailio5-mod-dialog +PACKAGE_kamailio5-mod-pua-dialoginfo:kamailio5-mod-pua
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Dialog Event PUA for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Dialog Event PUA for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pua-json
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-pua-json:libjson-c
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Presence user agent implementation with JSON messages for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Presence user agent implementation with JSON messages for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pua-reginfo
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-pua-reginfo:kamailio5-mod-pua +PACKAGE_kamailio5-mod-pua-reginfo:kamailio5-mod-usrloc
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: PUA registration info for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: PUA registration info for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pua-rpc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-pua-rpc:kamailio5-mod-pua
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RPC extensions for PUA for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: RPC extensions for PUA for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pua-usrloc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-pua-usrloc:kamailio5-mod-pua +PACKAGE_kamailio5-mod-pua-usrloc:kamailio5-mod-usrloc
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: PUA User Location for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: PUA User Location for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pua-xmpp
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-pua-xmpp:kamailio5-mod-presence +PACKAGE_kamailio5-mod-pua-xmpp:kamailio5-mod-pua +PACKAGE_kamailio5-mod-pua-xmpp:kamailio5-mod-xmpp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: PUA XMPP for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: PUA XMPP for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pv
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Pseudo-Variables for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Pseudo-Variables for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-pv-headers
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Flexible SIP header management for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Flexible SIP header management for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-qos
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-qos:kamailio5-mod-dialog
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: QoS control for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: QoS control for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ratelimit
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Traffic shapping for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Traffic shapping for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-regex
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-regex:libpcre
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Regular Expression for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Regular Expression for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-registrar
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-registrar:kamailio5-mod-usrloc
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP Registrar for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP Registrar for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-rls
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-rls:kamailio5-mod-presence +PACKAGE_kamailio5-mod-rls:kamailio5-mod-pua +PACKAGE_kamailio5-mod-rls:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Resource List Server for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Resource List Server for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-rr
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Record-Route and Route for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Record-Route and Route for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-rtimer
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Routing Timer for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Routing Timer for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-rtjson
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP routing based on JSON API for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP routing based on JSON API for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-rtpengine
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-rtpengine:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RTP engine for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: RTP engine for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-rtpproxy
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-rtpproxy:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RTP proxy for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: RTP proxy for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-sanity
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-sanity:kamailio5-mod-sl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP sanity checks for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP sanity checks for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-sca
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-sca:kamailio5-mod-sl +PACKAGE_kamailio5-mod-sca:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Shared Call Appearances for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Shared Call Appearances for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-sctp
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-sctp:libsctp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SCTP support for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SCTP support for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-sdpops
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Managing SDP payloads for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Managing SDP payloads for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-seas
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-seas:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Sip Express Application Server for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Sip Express Application Server for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-secfilter
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Allow/block filters for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Allow/block filters for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-sipcapture
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP capture for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP capture for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-sipdump
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Save SIP traffic for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Save SIP traffic for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-sipt
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP-T and SIP-I operations for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP-T and SIP-I operations for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-siptrace
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP trace for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP trace for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-siputils
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-siputils:kamailio5-mod-sl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP utilities for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP utilities for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-sl
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Stateless replier for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Stateless replier for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-sms
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-sms:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP-to-SMS IM gateway for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP-to-SMS IM gateway for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-smsops
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Handle SMS packets in SIP for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Handle SMS packets in SIP for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-snmpstats
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-snmpstats:libnetsnmp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SNMP interface for statistics for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SNMP interface for statistics for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-speeddial
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Per-user speed-dial controller for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Per-user speed-dial controller for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-sqlops
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SQL operations for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SQL operations for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-ss7ops
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: JSON Operations for SS7 over HEP for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: JSON Operations for SS7 over HEP for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-statistics
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Script statistics for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Script statistics for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-statsc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Statistics collector for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Statistics collector for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-statsd
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Connector for statsd application for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Connector for statsd application for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-stun
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: STUN server support for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: STUN server support for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-sst
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-sst:kamailio5-mod-dialog +PACKAGE_kamailio5-mod-sst:kamailio5-mod-sl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP Session Timer for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP Session Timer for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-tcpops
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: TCP options tweaking operations for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: TCP options tweaking operations for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-textops
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Text operations for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Text operations for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-textopsx
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Extra text operations for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Extra text operations for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-timer
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Execute routing blocks on core timers for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Execute routing blocks on core timers for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-tls
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-tls:libopenssl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: TLS operations for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: TLS operations for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-tmrec
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Match time recurrences for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Match time recurrences for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-topoh
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-topoh:kamailio5-mod-rr
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Topology hiding for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Topology hiding for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-topos
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-topos:kamailio5-mod-rr
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Topology stripping module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Topology stripping module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-topos-redis
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-topos-redis:kamailio5-mod-ndb-redis +PACKAGE_kamailio5-mod-topos-redis:kamailio5-mod-topos
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Redis backend for topos module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Redis backend for topos module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-tm
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Transaction for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Transaction for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-tmx
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Transaction module extensions for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Transaction module extensions for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-tsilo
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-tsilo:kamailio5-mod-registrar +PACKAGE_kamailio5-mod-tsilo:kamailio5-mod-sl +PACKAGE_kamailio5-mod-tsilo:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Transaction storage for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Transaction storage for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-uac
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-uac:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: User Agent Client for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: User Agent Client for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-uac-redirect
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-uac-redirect:kamailio5-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: User Agent Client redirection for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: User Agent Client redirection for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-uid-auth-db
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-uid-auth-db:kamailio5-mod-auth
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Authentication module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Authentication module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-uid-avp-db
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: AVP database operations for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: AVP database operations for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-uid-domain
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Domains management for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Domains management for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-uid-gflags
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Global attributes and flags for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Global attributes and flags for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-uid-uri-db
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Database URI operations for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Database URI operations for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-uri-db
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Database-backend SIP URI checking for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Database-backend SIP URI checking for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-userblacklist
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-userblacklist:kamailio5-lib-libtrie
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: User blacklists for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: User blacklists for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-usrloc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: User location for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: User location for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-utils
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-utils:libcurl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Misc utilities for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Misc utilities for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-uuid
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-uuid:libuuid
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: UUID utilities for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: UUID utilities for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-websocket
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-websocket:kamailio5-mod-sl +PACKAGE_kamailio5-mod-websocket:kamailio5-mod-tm +PACKAGE_kamailio5-mod-websocket:libopenssl +PACKAGE_kamailio5-mod-websocket:libunistring
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: WebSocket transport layer for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: WebSocket transport layer for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-xcap-client
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-xcap-client:libcurl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: XCAP Client for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: XCAP Client for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-xcap-server
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-xcap-server:kamailio5-mod-xhttp +PACKAGE_kamailio5-mod-xcap-server:kamailio5-mod-sl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: XCAP server implementation for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: XCAP server implementation for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-xhttp
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-xhttp:kamailio5-mod-sl
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Basic HTTP request handling server for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Basic HTTP request handling server for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-xhttp-pi
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-xhttp-pi:kamailio5-mod-xhttp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: HTTP provisioning interface for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: HTTP provisioning interface for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-xhttp-prom
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-xhttp-prom:kamailio5-mod-xhttp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Prometheus metrics for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Prometheus metrics for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-xhttp-rpc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-xhttp-rpc:kamailio5-mod-xhttp
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: RPC commands handling over HTTP for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: RPC commands handling over HTTP for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-xlog
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Advanced logger for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Advanced logger for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-xmlops
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: XML operations for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: XML operations for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-xmlrpc
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: XML RPC module for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: XML RPC module for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-xmpp
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5 +PACKAGE_kamailio5-mod-xmpp:kamailio5-mod-tm +PACKAGE_kamailio5-mod-xmpp:libexpat
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: SIP-to-XMPP Gateway for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: SIP-to-XMPP Gateway for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio5-mod-xprint
Submenu: Telephony
Version: 5.4.0-1
Depends: +libc +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio5
Conflicts: 
Menu-Depends: 
Provides: 
Section: net
Category: Network
Repository: base
Title: Print messages with specifiers for Kamailio5
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-5.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Print messages with specifiers for Kamailio5
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@


