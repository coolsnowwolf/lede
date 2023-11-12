local api = require "luci.passwall.api"
local appname = api.appname

m = Map(appname)
api.set_apply_on_parse(m)

-- [[ App Settings ]]--
s = m:section(TypedSection, "global_app", translate("App Update"),
				"<font color='red'>" ..
				translate("Please confirm that your firmware supports FPU.") ..
				"</font>")
s.anonymous = true
s:append(Template(appname .. "/app_update/app_version"))

local k, v
local com = require "luci.passwall.com"
for k, v in pairs(com) do
	o = s:option(Value, k:gsub("%-","_") .. "_file", translatef("%s App Path", v.name))
	o.default = v.default_path or ("/usr/bin/" .. k)
	o.rmempty = false
end

o = s:option(DummyValue, "tips", " ")
o.rawhtml = true
o.cfgvalue = function(t, n)
	return string.format('<font color="red">%s</font>', translate("if you want to run from memory, change the path, /tmp beginning then save the application and update it manually."))
end

return m
