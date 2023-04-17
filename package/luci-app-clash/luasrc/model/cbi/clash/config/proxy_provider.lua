
local m, s, o
local clash = "clash"
local uci = luci.model.uci.cursor()
local fs = require "nixio.fs"
local sys = require "luci.sys"
local sid = arg[1]
local http = luci.http
local fss = require "luci.clash"

function IsYamlFile(e)
   e=e or""
   local e=string.lower(string.sub(e,-5,-1))
   return e == ".yaml"
end
function IsYmlFile(e)
   e=e or""
   local e=string.lower(string.sub(e,-4,-1))
   return e == ".yml"
end


m = Map(clash, translate("Edit Proxy Provider"))
m.pageaction = false
m.redirect = luci.dispatcher.build_url("admin/services/clash/config/create")
if m.uci:get(clash, sid) ~= "proxyprovider" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, sid, "proxyprovider")
s.anonymous = true
s.addremove   = false

o = s:option(ListValue, "type", translate("Provider Type"))
o.rmempty = false
o.description = translate("Provider Type")
o:value("http")
o:value("file")

o = s:option(FileUpload, "",translate("Upload Provider File"))
o.title = translate("Provider File")
o.template = "clash/clash_upload"
o:depends("type", "file")
um = s:option(DummyValue, "", nil)
um.template = "clash/clash_dvalue"

local dir, fd
dir = "/etc/clash/proxyprovider/"
http.setfilehandler(
	function(meta, chunk, eof)
		if not fd then
			if not meta then return end

			if	meta and chunk then fd = nixio.open(dir .. meta.file, "w") end

			if not fd then
				um.value = translate("upload file error.")
				return
			end
		end
		if chunk and fd then
			fd:write(chunk)
		end
		if eof and fd then
			fd:close()
			fd = nil
			um.value = translate("File saved to") .. ' "/etc/clash/proxyprovider/"'
		end
	end
)

if luci.http.formvalue("upload") then
	local f = luci.http.formvalue("ulfile")
	if #f <= 0 then
		um.value = translate("No specify upload file.")
	end
end



o = s:option(Value, "name", translate("Provider Name"))
o.rmempty = false


o = s:option(ListValue, "path", translate("Provider Path"))
o.description = translate("Upload Provider File If Empty")
local p,h={}
for t,f in ipairs(fss.glob("/etc/clash/proxyprovider/*"))do
	h=fss.stat(f)
	if h then
    p[t]={}
    p[t].name=fss.basename(f)
    if IsYamlFile(p[t].name) or IsYmlFile(p[t].name) then
       o:value("./proxyprovider/"..p[t].name)
    end
  end
end
o.rmempty = true
o:depends("type", "file")


o = s:option(Value, "provider_url", translate("Provider URL"))
o.description = translate("【HTTP】./hk.yaml")
o.rmempty = true
o:depends("type", "http")

o = s:option(Value, "provider_interval", translate("Provider Interval"))
o.default = "3600"
o.rmempty = true
o:depends("type", "http")

o = s:option(ListValue, "health_check", translate("Provider Health Check"))
o:value("false", translate("Disable"))
o:value("true", translate("Enable"))
o.default=true

o = s:option(Value, "health_check_url", translate("Health Check URL"))
o.default = "http://www.gstatic.com/generate_204"
o.rmempty = true

o = s:option(Value, "health_check_interval", translate("Health Check Interval"))
o.default = "300"
o.rmempty = true

o = s:option(DynamicList, "pgroups", translate("Policy Group"))
o.rmempty = true
m.uci:foreach("clash", "pgroups",
		function(s)
			if s.name ~= "" and s.name ~= nil and s.type ~= "relay" then
			   o:value(s.name)
			end
		end)


local t = {
    {Apply, Return}
}

b = m:section(Table, t)

o = b:option(Button,"Apply")
o.inputtitle = translate("Save & Apply")
o.inputstyle = "apply"
o.write = function()
  m.uci:commit("clash")
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
