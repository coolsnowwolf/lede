--Alex<1886090@gmail.com>
local fs = require "nixio.fs"
local CONFIG_FILE = "/etc/pdnsd_gfw.cfg";

function sync_value_to_file(value, file)
	value = value:gsub("\r\n?", "\n")
	local old_value = nixio.fs.readfile(file)
	if value ~= old_value then
		nixio.fs.writefile(file, value)
	end

end
local state_msg = "" 

local pdnsd_on = string.len(luci.sys.exec("cat /var/run/pdnsd.pid"))>0
local other_pdnsd = (luci.sys.call("pidof pdnsd > /dev/null") == 0)

local resolv_file = luci.sys.exec("uci get dhcp.@dnsmasq[0].resolvfile")
if pdnsd_on then	
	state_msg = "<b><font color=\"green\">" .. translate("Running") .. "</font></b>"
else
	state_msg = "<b><font color=\"red\">" .. translate("Not running") .. "</font></b>"
end
local listen_port = luci.sys.exec("uci get dhcp.@dnsmasq[0].server")
if pdnsd_on and string.sub(listen_port,1,14) == "127.0.0.1#5053" then 
	state_msg=state_msg .. "，并恭喜你DNS转发正常"
elseif pdnsd_on then
	state_msg=state_msg .. "<b><font color=\"red\">但是当前DNS转发为｛" .. listen_port .."｝请到【DHCP/DNS】修改为【127.0.0.1#5353】，否则Pdnsd无效果</font></b>"
elseif other_pdnsd then
	state_msg=state_msg .. "，但是有其他程序开启了Pdnsd，如ShadowsocksR"
end
if resolv_file=="" then

else if pdnsd_on then
	state_msg=state_msg .. "<b><font color=\"red\">请到【DHCP/DNS】【HOSTS和解析文件】勾上【忽略解析文件】的勾，否则Pdnsd可能无效果</font></b>"
	end
end
m=Map("pdnsd",translate("Pdnsd"),translate("Pdnsd可以通过TCP协议进行DNS解析,可以方便的使用iptables进行透明代理，配合ipset、GFWList使用效果更佳。默认上游服务器为114DNS，可以在【DHCP/DNS】中设置【DNS转发】为【127.0.0.1#5353】即可将全局DNS请求交给pdnsd进行解析，由于pdnsd自带缓存，所以可以加快解析速度。另外你可以使用dig来检测DNS解析情况，方法dig @127.0.0.1 -p 5053 www.facebook.com ).. "<br><br>状态 - " .. state_msg)
s=m:section(TypedSection,"arguments","")
s.addremove=false
s.anonymous=true
	view_enable = s:option(Flag,"enabled",translate("Enable"))
	view_cfg = s:option(TextValue, "1", nil)
	view_cfg.rmempty = false
	view_cfg.rows = 43

	function view_cfg.cfgvalue()
		return nixio.fs.readfile(CONFIG_FILE) or ""
	end
	function view_cfg.write(self, section, value)
		sync_value_to_file(value, CONFIG_FILE)
	end
-- ---------------------------------------------------
local apply = luci.http.formvalue("cbi.apply")
if apply then
	os.execute("/etc/pdnsd_init.sh restart >/dev/null 2>&1 &")
end

return m
