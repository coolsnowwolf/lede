

local fs = require "nixio.fs"
local china_file = "/etc/ssrr/china_route"
local user_local_file = "/etc/ssrr/user_local_ip"
local user_remote_file = "/etc/ssrr/user_remote_ip"

function sync_value_to_file(value, file)
	value = value:gsub("\r\n?", "\n")
	local old_value = nixio.fs.readfile(file)
	if value ~= old_value then
		nixio.fs.writefile(file, value)
	end
end

m = Map("ssrr", translate("路由表"),translate("指定国内外路由表，并且可以设置强制走和不走的网段"))
s = m:section(TypedSection, "shadowsocksr", translate("Settings"))
s.anonymous = true

button_update_route = s:option (Button, "_button_update_chinaroute", translate("更新国内路由表"),translate("点击后请静待30秒,如非特殊需要，不用更新该表")) 
local route_count = luci.sys.exec("grep -c '' " .. china_file)
button_update_route.inputtitle = translate ( "当前规则数目" .. route_count .. ",点击更新")
button_update_route.inputstyle = "apply" 
function button_update_route.write (self, section, value)
	luci.sys.call ( "nohup sh /etc/ssrr/update_chinaroute.sh > /tmp/gfwupdate.log 2>&1 &")
end 



china_route = s:option(TextValue, "china_route", translate("国内IP网段"), nil)
china_route.description = translate("该列表是国内外分流的主要依据，内容会随着更新而被覆盖")
china_route.rows = 13
china_route.wrap = "off"
china_route.cfgvalue = function(self, section)
	return fs.readfile(china_file) or ""
end
china_route.write = function(self, section, value)
	fs.writefile(china_file, value:gsub("\r\n", "\n"))
end

user_local = s:option(TextValue, "user_local", translate("强制不走代理的网段"), nil)
user_local.description = translate("请不要随意删除，请填写内网网段")
user_local.rows = 13
user_local.wrap = "off"
user_local.cfgvalue = function(self, section)
	return fs.readfile(user_local_file) or ""
end
user_local.write = function(self, section, value)
	fs.writefile(user_local_file, value:gsub("\r\n", "\n"))
end

user_remote = s:option(TextValue, "user_remote", translate("强制走代理的网段"), nil)
user_remote.description = translate("该规则优先权低于强制不走IP的网段,一般需要填写telegram这样软件服务器的IP")
user_remote.rows = 13
user_remote.wrap = "off"
user_remote.cfgvalue = function(self, section)
	return fs.readfile(user_remote_file) or ""
end
user_remote.write = function(self, section, value)
	fs.writefile(user_remote_file, value:gsub("\r\n", "\n"))
end

-- ---------------------------------------------------
local apply = luci.http.formvalue("cbi.apply")
if apply then
	os.execute("/etc/init.d/ssrr restart >/dev/null 2>&1 &")
end

return m
