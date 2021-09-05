-- Copyright 2017 Yousong Zhou <yszhou4tech@gmail.com>
-- Licensed to the public under the Apache License 2.0.

local ds = require "luci.dispatcher"
local ss = require "luci.model.shadowsocks-libev"
local ut = require "luci.util"
local m, s, o

m = Map("shadowsocks-libev",
	translate("Local Instances"),
	translate("Instances of shadowsocks-libev components, e.g. ss-local, \
			   ss-redir, ss-tunnel, ss-server, etc.  To enable an instance it \
			   is required to enable both the instance itself and the remote \
			   server it refers to."))

local instances = {}
local cfgtypes = { "ss_local", "ss_redir", "ss_server", "ss_tunnel" }

for sname, sdata in pairs(m:get()) do
	local key, value = ss.cfgvalue_overview(sdata)
	if key ~= nil then
		instances[key] = value
	end
end

s = m:section(Table, instances)
s.addremove = true
s.template_addremove = "shadowsocks-libev/add_instance"
s.extedit = function(self, section)
	local value = instances[section]
	if type(value) == "table" then
		return ds.build_url(unpack(ds.context.requestpath),
					"services/shadowsocks-libev/instances",
					value[".name"])
	end
end
s.parse = function(self, ...)
	Table.parse(self, ...)

	local crval = REMOVE_PREFIX .. self.config
	local name = self.map:formvaluetable(crval)
	for k,v in pairs(name) do
		local value = instances[k]
		local sname = value[".name"]
		if type(value) == "table" then
			m:del(sname)
			instances[k] = nil
			for _, oname in ipairs({"redir_tcp", "redir_udp"}) do
				local ovalue = m:get("ss_rules", oname)
				if ovalue == sname then
					m:del("ss_rules", oname)
				end
			end
		end
	end

	local stype = m:formvalue("_newinst.type")
	local sname = m:formvalue("_newinst.name")
	if ut.contains(cfgtypes, stype) then
		local created
		if sname and #sname > 0 then
			created = m:set(sname, nil, stype)
		else
			created = m:add(stype)
			sname = created
		end
		if created then
			m.uci:save("shadowsocks-libev")
			luci.http.redirect(ds.build_url(
				"admin/services/shadowsocks-libev/instances", sname
			))
		end
	end
end

o = s:option(DummyValue, "name", translate("Name"))
o.rawhtml = true
o = s:option(DummyValue, "overview", translate("Overview"))
o.rawhtml = true

s:option(DummyValue, "running", translate("Running"))

o = s:option(Button, "disabled", translate("Enable/Disable"))
o.render = function(self, section, scope)
	if instances[section].disabled then
		self.title = translate("Disabled")
		self.inputstyle = "reset"
	else
		self.title = translate("Enabled")
		self.inputstyle = "save"
	end
	Button.render(self, section, scope)
end
o.write = function(self, section)
	local sdata = instances[section]
	if type(sdata) == "table" then
		local sname = sdata[".name"]
		local disabled = not sdata["disabled"]
		sdata["disabled"] = disabled
		m:set(sname, "disabled", tostring(disabled))
	end
end

return m
