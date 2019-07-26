
mp = Map("unblockmusic", translate("恢复网易云音乐无版权歌曲"))
mp.description = translate("原理：采用 [网易云旧链/QQ/虾米/百度/酷狗/酷我/咕咪/JOOX] 等音源，替换网易云音乐无版权歌曲链接<br/>具体使用方法参见：https://github.com/project-openwrt/luci-app-unblockmusic")

mp:section(SimpleSection).template  = "unblockmusic/unblockmusic_status"

s = mp:section(TypedSection, "unblockmusic")
s.anonymous=true
s.addremove=false

enabled = s:option(Flag, "enabled", translate("启用UnblockNeteaseMusic"))
enabled.description = translate("启用本插件以恢复网易云音乐无版权歌曲")
enabled.default = 0
enabled.rmempty = false

enabled = s:option(Flag, "strict_mode", translate("启用严格模式"))
enabled.description = translate("若将服务部署到公网，则强烈建议使用严格模式，此模式下仅放行网易云音乐所属域名的请求")
enabled.default = 1
enabled.rmempty = false
enabled:depends("enabled", 1)

enabled = s:option(Flag, "enable_ipset_forward", translate("启用IPSet转发相关流量"))
enabled.description = translate("开启后，所有网易云音乐流量都会被重定向到UnblockNeteaseMusic进行处理，但会导致无法正常访问云音乐网页端")
enabled.default = 1
enabled.rmempty = false
enabled:depends("enabled", 1)

speedtype = s:option(ListValue, "musicapptype", translate("音源选择"))
speedtype:value("default", translate("默认"))
speedtype:value("netease", translate("网易云音乐"))
speedtype:value("qq", translate("QQ音乐"))
speedtype:value("xiami", translate("虾米音乐"))
speedtype:value("baidu", translate("百度音乐"))
speedtype:value("kugou", translate("酷狗音乐"))
speedtype:value("kuwo", translate("酷我音乐"))
speedtype:value("migu", translate("咕咪音乐"))
speedtype:value("joox", translate("JOOX音乐"))
speedtype:value("all", translate("所有平台"))
speedtype.description = translate("音源调用接口")
speedtype.default = "all"
speedtype:depends("enabled", 1)

account = s:option(Value, "port", translate("监听端口"))
account.description = translate("UnblockNeteaseMusic监听的端口，不可与其他程序共用一个端口")
account.default = "5200"
account.datatype = "string"
account:depends("enabled", 1)

enabled = s:option(Flag, "set_netease_server_ip", translate("自定义网易云服务器IP"))
enabled.description = translate("如使用Hosts方式则必选，否则将会导致连接死循环")
enabled.default = 0
enabled.rmempty = false
enabled:depends("enabled", 1)

account = s:option(Value, "netease_server_ip", translate("网易云服务器IP"))
account.description = translate("通过 ping music.163.com 即可获得IP地址，仅限填写一个")
account.default = "59.111.181.38"
account.datatype = "string"
account:depends("set_netease_server_ip", 1)

enabled = s:option(Flag, "enable_proxy", translate("使用代理服务器"))
enabled.description = translate("如您的OpenWRT系统部署在海外，则此选项必选，否则可能无法正常使用")
enabled.default = 0
enabled.rmempty = false
enabled:depends("enabled", 1)

account = s:option(Value, "proxy_server_ip", translate("代理服务器IP"))
account.description = translate("具体格式请参考：https://github.com/nondanee/UnblockNeteaseMusic")
account.datatype = "string"
account:depends("enable_proxy", 1)

return mp
