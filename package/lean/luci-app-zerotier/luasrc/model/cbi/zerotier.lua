local e=require"nixio.fs"
local e=luci.http
local o=require"luci.model.network".init()
local a,t,e,b
a=Map("zerotier",translate("ZeroTier"),translate("ZeroTier 是一款可以创建P2P虚拟局域网的开源软件。"))
a:section(SimpleSection).template  = "zerotier/zerotier_status"
t=a:section(NamedSection,"sample_config","zerotier",translate("全局设置"))
t.anonymous=true
t.addremove=false
e=t:option(Flag,"enabled",translate("启用"))
e.default=0
e.rmempty=false
e=t:option(ListValue,"interface",translate("接口"))
for b,t in ipairs(o:get_networks())do
if t:name()~="loopback" then e:value(t:name())end
end
-- e=t:option(Value,"start_delay",translate("延时启动"),translate("Units:seconds"))
-- e.datatype="uinteger"
-- e.default="0"
-- e.rmempty=true
e=t:option(DynamicList,"join",translate('ZeroTier 网络 ID'))
e.password=true
e.rmempty=false
if nixio.fs.access("/etc/config/zerotier")then
e=t:option(Button,"Configuration",translate("网络配置管理"))
e.inputtitle=translate("打开网站")
e.inputstyle="reload"
e.write=function()
luci.http.redirect("https://my.zerotier.com/")
end
end
return a
