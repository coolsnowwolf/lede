-- Copyright 2014 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local proto = luci.model.network:register_protocol("hnet")

function proto.get_i18n(self)
	return luci.i18n.translate("Automatic Homenet (HNCP)")
end

function proto.is_installed(self)
	return nixio.fs.access("/lib/netifd/proto/hnet.sh")
end

function proto.opkg_package(self)
	return "hnet-full"
end
