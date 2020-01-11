-- Licensed to the public under the GNU General Public License v3.

local m, s, o
local shadowsocksr = "shadowsocksr"

local uci = luci.model.uci.cursor()
local server_count = 0
uci:foreach("shadowsocksr", "servers", function(s)
  server_count = server_count + 1
end)

local fs  = require "nixio.fs"
local sys = require "luci.sys"

m = Map(shadowsocksr,  translate("Servers subscription and manage"))

-- Server Subscribe

s = m:section(TypedSection, "server_subscribe")
s.anonymous = true

o = s:option(Flag, "auto_update", translate("Auto Update"))
o.rmempty = false
o.description = translate("Auto Update Server subscription, GFW list and CHN route")


o = s:option(ListValue, "auto_update_time", translate("Update time (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=2
o.rmempty = false

o = s:option(DynamicList, "subscribe_url", translate("Subscribe URL"))
o.rmempty = true

o = s:option(Button,"update_Sub",translate("Update Subscribe List"))
o.inputstyle = "reload"
o.description = translate("Update subscribe url list first")
o.write = function()
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "shadowsocksr", "servers"))
end

o = s:option(Flag, "proxy", translate("Through proxy update"))
o.rmempty = false
o.description = translate("Through proxy update list, Not Recommended ")

o = s:option(Button,"subscribe", translate("Update All Subscribe Severs"))
o.rawhtml  = true
o.template = "shadowsocksr/subscribe"


-- o.inputstyle = "apply"
-- o.write = function()
--   luci.sys.call("lua /root/subscribe.lua  >>/tmp/ssrplus.log 2>&1")
--   -- luci.sys.call("echo 123  >>/tmp/ssrplus.log 2>&1")
--    --luci.sys.exec("bash /usr/share/shadowsocksr/subscribe.sh >>/tmp/ssrplus.log 2>&1")
--    luci.http.redirect(luci.dispatcher.build_url("admin", "services", "shadowsocksr", "servers"))
-- end


o = s:option(Button,"delete",translate("Delete all severs"))
o.inputstyle = "reset"
o.description = string.format(translate("Server Count") ..  ": %d", server_count)
o.write = function()
  uci:delete_all("shadowsocksr", "servers", function(s) return true end)
  uci:save("shadowsocksr")
  luci.sys.call("uci commit shadowsocksr && /etc/init.d/shadowsocksr stop")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "shadowsocksr", "servers"))
  return
end

-- [[ Servers Manage ]]--
s = m:section(TypedSection, "servers")
s.anonymous = true
s.addremove = true
s.sortable = false
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/services/shadowsocksr/servers/%s")
function s.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(s.extedit % sid)
		return
	end
end

o = s:option(DummyValue, "type", translate("Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("")
end

o = s:option(DummyValue, "alias", translate("Alias"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue, "server", translate("Server Address"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "?"
end

o = s:option(DummyValue, "server_port", translate("Server Port"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "?"
end

if nixio.fs.access("/usr/bin/kcptun-client") then

o = s:option(DummyValue, "kcp_enable", translate("KcpTun"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "?"
end

end

o = s:option(DummyValue, "switch_enable", translate("Auto Switch"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or "0"
end

o = s:option(DummyValue, "server_port", translate("Socket Connected"))
o.template="shadowsocksr/socket"
o.width="10%"

o = s:option(DummyValue, "server", translate("Ping Latency"))
o.template="shadowsocksr/ping"
o.width="10%"

m:append(Template("shadowsocksr/server_list"))

return m
