-- Copyright 2013 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local proto = luci.model.network:register_protocol("dhcpv6")

function proto.get_i18n(self)
	return luci.i18n.translate("DHCPv6 client")
end

function proto.is_installed(self)
	return nixio.fs.access("/lib/netifd/proto/dhcpv6.sh")
end

function proto.opkg_package(self)
	return "odhcp6c"
end
