
local m, s, o
local clash = "clash"
local uci = luci.model.uci.cursor()
local fs = require "nixio.fs"
local sys = require "luci.sys"
local sid = arg[1]


m = Map(clash, translate("Edit Group"))
m.pageaction = false
m.redirect = luci.dispatcher.build_url("admin/services/clash/config/create")
if m.uci:get(clash, sid) ~= "pgroups" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Groups Setting ]]--
s = m:section(NamedSection, sid, "pgroups")
s.anonymous = true
s.addremove   = false

o = s:option(ListValue, "type", translate("Group Type"))
o.rmempty = true
o.description = translate("Choose The Operation Mode")
o:value("select", translate("Select"))
o:value("url-test", translate("URL-Test"))
o:value("fallback", translate("Fallback"))
o:value("load-balance", translate("Load-Balance"))
o:value("relay", translate("Relay"))

o = s:option(Value, "name", translate("Group Name"))
o.rmempty = false

o = s:option(Value, "test_url", translate("Test URL"))
o.default = "http://www.gstatic.com/generate_204"
o.rmempty = true
o:depends("type", "url-test")
o:depends("type", "fallback")
o:depends("type", "load-balance")

o = s:option(Value, "test_interval", translate("Test Interval(s)"))
o.default = "300"
o.rmempty = true
o:depends("type", "url-test")
o:depends("type", "fallback")
o:depends("type", "load-balance")

o = s:option(DynamicList, "other_group", translate("Other Group"))
o.rmempty = true
o.description = translate("Proxy Groups Must Exist In Rule")
o:value("ALL", translate("All Servers"))
uci:foreach("clash", "pgroups",
		function(s)
		  if s.name ~= "" and s.name ~= nil and s.name ~= m.uci:get(clash, sid, "name") then
			   o:value(s.name)
			end
		end)
uci:foreach("clash", "servers",
		function(s)
		  if s.name ~= "" and s.name ~= nil and s.name ~= m.uci:get(clash, sid, "name") then
			   o:value(s.name)
			end
		end)		
o:value("DIRECT")
o:value("REJECT")


local t = {
    {Apply, Return}
}

b = m:section(Table, t)

o = b:option(Button,"Apply")
o.inputtitle = translate("Save & Apply")
o.inputstyle = "apply"
o.write = function()
  m.uci:commit("clash")
  sys.call("/usr/share/clash/create/pgroups.sh start >/dev/null 2>&1 &")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "create"))
end

o = b:option(Button,"Return")
o.inputtitle = translate("Back to Overview")
o.inputstyle = "reset"
o.write = function()
   m.uci:revert(clash)
   luci.http.redirect(m.redirect)
  --luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "providers"))
end


return m
