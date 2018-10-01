

local m, s, o
local shadowsocksr = "shadowsocksr"

local function has_bin(name)
	return luci.sys.call("command -v %s >/dev/null" %{name}) == 0
end

m = Map(shadowsocksr, "%s - %s" %{translate("ShadowSocksR"), translate("Servers Manage")})

-- Server Subscribe
if nixio.fs.access("/usr/share/shadowsocksr/subscribe.sh") and has_bin("base64") and has_bin("curl") and has_bin("bash") and has_bin("dig") then
	s = m:section(TypedSection, "server_subscribe", translate("Server subscription"))
	s.anonymous = true

	o = s:option(Flag, "auto_update", translate("Auto Update"))
	o.rmempty = false

	o = s:option(Flag, "proxy", translate("Through proxy update"))
	o.rmempty = false

	o = s:option(ListValue, "auto_update_time", translate("Update time (every day)"))
	for t = 0,23 do
	o:value(t, t..":00")
	end
	o.default=2
	o.rmempty = false

	o = s:option(DynamicList, "subscribe_url", translate("Subscribe URL"))
	o.rmempty = true

	o = s:option(Button,"update",translate("Update"))
	o.write = function()
	luci.sys.call("/usr/share/shadowsocksr/subscribe.sh >/dev/null 2>&1")
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "shadowsocksr", "servers"))
	end
end

return m
