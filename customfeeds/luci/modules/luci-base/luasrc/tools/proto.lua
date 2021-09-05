-- Copyright 2012 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.tools.proto", package.seeall)

function opt_macaddr(s, ifc, ...)
	local v = luci.cbi.Value
	local o = s:taboption("advanced", v, "macaddr", ...)

	o.placeholder = ifc and ifc:mac()
	o.datatype    = "macaddr"

	function o.cfgvalue(self, section)
		local w = ifc and ifc:get_wifinet()
		if w then
			return w:get("macaddr")
		else
			return v.cfgvalue(self, section)
		end
	end

	function o.write(self, section, value)
		local w = ifc and ifc:get_wifinet()
		if w then
			w:set("macaddr", value)
		elseif value then
			v.write(self, section, value)
		else
			v.remove(self, section)
		end
	end

	function o.remove(self, section)
		self:write(section, nil)
	end
end
