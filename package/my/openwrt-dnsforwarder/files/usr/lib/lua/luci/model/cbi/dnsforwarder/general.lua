--Alex<1886090@gmail.com>
local fs = require "nixio.fs"
local CONFIG_FILE = "/etc/dnsforwarder/dnsforwarder.conf";

function sync_value_to_file(value, file)
	value = value:gsub("\r\n?", "\n")
	local old_value = nixio.fs.readfile(file)
	if value ~= old_value then
		nixio.fs.writefile(file, value)
	end

end
local state_msg = "" 

local dnsforwarder_on = (luci.sys.call("pidof dnsforwarder > /dev/null") == 0)
local resolv_file = luci.sys.exec("uci get dhcp.@dnsmasq[0].resolvfile")
local listen_port = luci.sys.exec("uci get dhcp.@dnsmasq[0].server")

if dnsforwarder_on then	
	state_msg = "<b><font color=\"green\">" .. translate("Running") .. "</font></b>"
else
	state_msg = "<b><font color=\"red\">" .. translate("Not running") .. "</font></b>"
end


if dnsforwarder_on and string.sub(listen_port,1,14) == "127.0.0.1#5053" then 
	state_msg=state_msg .. "，DNSmasq已经将流量定向至本软件"
elseif dnsforwarder_on then
	state_msg=state_msg .. "<b><font color=\"red\"></font></b>"
end

if resolv_file=="" then

else if dnsforwarder_on then
	state_msg=state_msg .. "<b><font color=\"red\"></font></b>"
	end
end
m=Map("dnsforwarder",translate("dnsforwarder"),translate("dnsforwarder是企业级DNS服务器，可以通过TCP协议进行DNS解析,可以方便的使用iptables进行透明代理，配合ipset、GFWList使用效果更佳。默认上游服务器为114DNS，SSR和Redsocks的GFWList模式要依赖本软件包提供的功能").. "<br><br>状态 - " .. state_msg)
s=m:section(TypedSection,"arguments","")
s.addremove=false
s.anonymous=true
	view_enable = s:option(Flag,"enabled",translate("Enable"))
	view_redir = s:option(Flag,"redir",translate("劫持所有DNS请求"),translate("自动将所有客户端的DNS请求都劫持到dnsforwarder的端口"))
	view_port = s:option(Value,"port",translate("dnsforwarder监听端口"),translate("请根据下面的配置文件填写端口，默认5053"))
	view_port:depends({redir=1}) 
	view_port.default="5053"
	view_port.datatype="uinteger"
	view_dnsmasq = s:option(Flag,"dnsmasq",translate("设置成DNSmasq的上游服务器"),translate("让DNSMasq从本软件获得解析结果，支持GFWList模式"))
	view_dnsmasq:depends({redir=0})
	view_mode=s:option(ListValue,"mode",translate("DNSmasq转发模式"),translate("dnsmasq可以将全部请求发送至本软件，也可以只发送GFWList登记部分，其余由系统默认DNS解析"))  
	view_mode:value("gfwlist",translate("只转发GFWList"))
	view_mode:value("userlist",translate("只转发用户自定义列表"))
	view_mode:value("gfw_user",translate("转发gfwlist+用户自定义列表"))
	view_mode:value("all",translate("全部转发给dnsforwarder"))  
	view_mode:depends({dnsmasq=1})
	view_addr = s:option(Value,"addr",translate("转发地址"),translate("请填写dnsforwarder的监听地址,默认127.0.0.1:5053,如果填写<b><font color=\"red\">208.67.222.222:5353</font></b>那么可不通过该软件获得无污染结果"))
	view_addr:depends({dnsmasq=1})
	view_addr.default = "127.0.0.1:5053"
	view_ipset = s:option(Flag,"ipset",translate("将查询结果放入ipset"),translate("为SSR，Redsocks的GFWList模式提供依据"))
	view_ipset:depends({mode="userlist"})
	view_ipset:depends({mode="gfwlist"})
	view_ipset:depends({mode="gfw_user"})
	view_ipsetname = s:option(Value,"ipset_name",translate("ipset名字"))
	view_ipsetname.default = "china-banned"
	view_ipsetname:depends({ipset=1})

	view_white = s:option(Flag,"white",translate("启用强制不走代理列表"),translate("配合SSR使用，设置某些域名不走代理"))
	view_white:depends({mode="userlist"})
	view_white:depends({mode="gfwlist"})
	view_white:depends({mode="gfw_user"})
	whitedns = s:option(Value,"whitedns",translate("白名单专用DNS"),translate("白名单内的列表都会用此DNS进行解析，并由DNSMasq转发，最终不走SSR代理"))
	whitedns.default="114.114.114.114:53"
	whitedns:depends({white=1})
	whiteipset = s:option(Value,"whiteset",translate("白名单ipset"),translate("设置收集到的白名单的域名的ipset列表名字"))
	whiteipset:depends({white=1})
	whiteipset.default="whiteset"
	


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
	os.execute("/etc/init.d/dnsforwarder restart >/dev/null 2>&1 &")
end

return m
