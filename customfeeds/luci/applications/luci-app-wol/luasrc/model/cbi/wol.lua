-- Copyright 2010 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys = require "luci.sys"
local fs  = require "nixio.fs"

m = SimpleForm("wol", translate("Wake on LAN"),
	translate("Wake on LAN is a mechanism to remotely boot computers in the local network."))

m.submit = translate("Wake up host")
m.reset  = false


local has_ewk = fs.access("/usr/bin/etherwake")
local has_wol = fs.access("/usr/bin/wol")


s = m:section(SimpleSection)

if has_ewk and has_wol then
	bin = s:option(ListValue, "binary", translate("WoL program"),
		translate("Sometimes only one of the two tools works. If one fails, try the other one"))

	bin:value("/usr/bin/etherwake", "Etherwake")
	bin:value("/usr/bin/wol", "WoL")
end

if has_ewk then
	iface = s:option(ListValue, "iface", translate("Network interface to use"),
		translate("Specifies the interface the WoL packet is sent on"))

	if has_wol then
		iface:depends("binary", "/usr/bin/etherwake")
	end

	iface:value("", translate("Broadcast on all interfaces"))
	
	for _, e in ipairs(sys.net.devices()) do
		if e ~= "lo" then iface:value(e) end
	end
	
	iface.default="br-lan"
end


host = s:option(Value, "mac", translate("Host to wake up"),
	translate("Choose the host to wake up or enter a custom MAC address to use"))

sys.net.mac_hints(function(mac, name)
	host:value(mac, "%s (%s)" %{ mac, name })
end)


function host.write(self, s, val)
	local host = luci.http.formvalue("cbid.wol.1.mac")
	if host and #host > 0 and host:match("^[a-fA-F0-9:]+$") then
		local cmd
		local util = luci.http.formvalue("cbid.wol.1.binary") or (
			has_ewk and "/usr/bin/etherwake" or "/usr/bin/wol"
		)

		if util == "/usr/bin/etherwake" then
			local iface = luci.http.formvalue("cbid.wol.1.iface")
			cmd = "%s -D%s %q" %{
				util, (iface ~= "" and " -i %q" % iface or ""), host
			}
		else
			cmd = "%s -v %q" %{ util, host }
		end

		local msg = "<p><strong>%s</strong><br /><br /><code>%s<br /><br />" %{
			translate("Starting WoL utility:"), cmd
		}

		local p = io.popen(cmd .. " 2>&1")
		if p then
			while true do
				local l = p:read("*l")
				if l then
					if #l > 100 then l = l:sub(1, 100) .. "..." end
					msg = msg .. l .. "<br />"
				else
					break
				end
			end
			p:close()
		end

		msg = msg .. "</code></p>"

		m.message = msg
	end
end


return m