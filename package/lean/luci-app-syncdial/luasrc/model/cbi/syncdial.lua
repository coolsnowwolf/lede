local e=require"nixio.fs"
require("luci.tools.webadmin")
local e="mwan3 status | grep -c \"is online and tracking is active\""
local e=io.popen(e,"r")
local t=e:read("*a")
e:close()
m=Map("syncdial",translate("多线多拨"),
translate("使用macvlan驱动创建多个虚拟WAN口，支持并发多拨 <br />当前在线接口数量：")..t)
s=m:section(TypedSection,"syncdial",translate(" "))
s.anonymous=true
o=s:option(Flag,"enabled","启用")
o.rmempty=false
o=s:option(Flag,"syncon","启用并发多拨")
o.rmempty=false
o=s:option(ListValue,"dial_type",translate("多拨类型"))
o:value("1",translate("单线多拨"))
o:value("2",translate("双线多拨"))
o.rmempty=false
o=s:option(Value,"wanselect",translate("选择外网接口"),translate("指定要多拨的外网接口，如wan"))
luci.tools.webadmin.cbi_add_networks(o)
o.optional=false
o.rmempty=false
o=s:option(Value,"wannum","虚拟WAN接口数量")
o.datatype="range(0,249)"
o.optional=false
o.default=1
o=s:option(Flag,"bindwan","绑定物理接口")
o.rmempty=false
o=s:option(Value,"wanselect2",translate("选择第二个外网接口"),translate("<font color=\"red\">指定要多拨的第二个外网接口，如wan2</font>"))
luci.tools.webadmin.cbi_add_networks(o)
o.optional=false
o:depends("dial_type","2")
o=s:option(Value,"wannum2",translate("第二条线虚拟WAN接口数量"),translate("设置第二条线的拨号数"))
o.datatype="range(0,249)"
o.optional=false
o.default=1
o:depends("dial_type","2")
o=s:option(Flag,"bindwan2","绑定物理接口","第二条线生成的虚拟接口绑定当前物理接口")
o.rmempty=false
o:depends("dial_type","2")
o=s:option(Flag,"dialchk","启用掉线检测")
o.rmempty=false
o=s:option(Value,"dialnum","最低在线接口数量","如果在线接口数量小于这个值则重拨。")
o.datatype="range(0,248)"
o.optional=false
o.default=2
o=s:option(Value,"dialnum2","第二条线最低在线接口数量","如果第二条线在线接口数量小于这个值则重拨。")
o.datatype="range(0,248)"
o.optional=false
o.default=2
o:depends("dial_type","2")
o=s:option(Value,"dialwait","重拨等待时间","重拨时，接口全部下线后下一次拨号前的等待时间。单位：秒 最小值：5秒")
o.datatype="and(uinteger,min(5))"
o.optional=false
o=s:option(Flag,"old_frame","使用旧的macvlan创建方式")
o.rmempty=false
o=s:option(Flag,"nomwan","不自动配置MWAN3负载均衡","需要自定义负载均衡设置或者要使用策略路由的用户选择")
o.rmempty=false
o=s:option(DummyValue,"_redial","重新并发拨号")
o.template="syncdial/redial_button"
o.width="10%"
return m
