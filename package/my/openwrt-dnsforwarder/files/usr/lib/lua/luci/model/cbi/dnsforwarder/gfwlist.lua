local fs = require "nixio.fs"
local banned = "/etc/dnsforwarder/china-banned"
local user = "/etc/dnsforwarder/userlist"
local whitedomin = "/etc/dnsforwarder/whitelist"
function sync_value_to_file(value, file)
	value = value:gsub("\r\n?", "\n")
	local old_value = nixio.fs.readfile(file)
	if value ~= old_value then
		nixio.fs.writefile(file, value)
	end
end

m = Map("dnsforwarder", translate("Domain Lists Settings"),translate("‘GFWList黑名单’可以观察当前域名黑名单，更新后会发生变化;<br>‘用户自定义黑名单’不会被GFWList更新所覆盖，可以手动添加一些强制走代理的网站。<br>注意：点击更新按钮后需要等浏览器自动刷新后才算完成，请勿重复点击"))
s=m:section(TypedSection,"arguments","")
s.addremove=false
s.anonymous=true

button_update_gfwlist = s:option (Button, "_button_update_gfwlist", translate("更新GFWList"),translate("点击后请静待30秒,然后刷新本页面")) 
local gfw_count = luci.sys.exec("grep -c '' " .. banned)
button_update_gfwlist.inputtitle = translate ( "当前规则数目" .. gfw_count .. ",点击更新")
button_update_gfwlist.inputstyle = "apply" 
function button_update_gfwlist.write (self, section, value)
	luci.sys.call("nohup sh /etc/dnsforwarder/up-gfwlist.sh > /tmp/gfwupdate.log 2>&1 &")
end 


gfwlist = s:option(TextValue, "gfwlist", translate("被GFW屏蔽的名单"), nil)
gfwlist.description = translate("该列表内的内容会随着GFWList更新而覆盖")
gfwlist.rows = 13
gfwlist.wrap = "off"
gfwlist.cfgvalue = function(self, section)
	return fs.readfile(banned) or ""
end
gfwlist.write = function(self, section, value)
	fs.writefile(banned, value:gsub("\r\n", "\n"))
end

userlist = s:option(TextValue, "users", translate("用户自定义黑名单"), nil)
userlist.description = translate("该列表的内容不会随着GFWList的更新而改变")
userlist.rows = 13
userlist.wrap = "off"
userlist.cfgvalue = function(self, section)
	return fs.readfile(user) or ""
end
userlist.write = function(self, section, value)
	fs.writefile(user, value:gsub("\r\n", "\n"))
end

goods = s:option(TextValue, "fafa", translate("强制不走代理白名单"), nil)
goods.description = translate("一般填写国内的CDN服务器地址，需配合SSR的GFWList自动代理模式使用")
goods.rows = 13
goods.wrap = "off"
goods.cfgvalue = function(self, section)
	return fs.readfile(whitedomin) or ""
end
goods.write = function(self, section, value)
	fs.writefile(whitedomin, value:gsub("\r\n", "\n"))
end




-- ---------------------------------------------------
local apply = luci.http.formvalue("cbi.apply")
if apply then
	os.execute("/etc/init.d/dnsforwarder restart >/dev/null 2>&1 &")
end

return m