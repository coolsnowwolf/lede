--[[

Copyright (C) 2020 KFERMercer <KFER.Mercer@gmail.com>
Copyright (C) 2020 [CTCGFW] Project OpenWRT

THIS IS FREE SOFTWARE, LICENSED UNDER GPLv3

]]--

m = Map("baidupcs-web")
m.title	= translate("BaiduPCS-Web")
m.description = translate("基于BaiduPCS-Go, 可以让你高效的使用百度云")

m:section(SimpleSection).template  = "baidupcs-web/baidupcs-web_status"

s = m:section(TypedSection, "baidupcs-web")
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enabled", translate("启用"))
enable.rmempty = false

o = s:option(Value, "port", translate("网页端口"))
o.datatype = "port"
o.placeholder = "5299"
o.default = "5299"
o.rmempty = false

o = s:option(Value, "download_dir", translate("下载目录"))
o.placeholder = "/opt/baidupcsweb-download"
o.default = "/opt/baidupcsweb-download"
o.rmempty = false

o = s:option(Value, "max_download_rate", translate("最大下载速度"))
o.placeholder = "0"

o = s:option(Value, "max_upload_rate", translate("最大上传速度"))
o.placeholder = "0"
o.description = translate("0代表不限制, 单位为每秒的传输速率, 后缀'/s' 可省略, 如 2MB/s, 2MB, 2m, 2mb 均为一个意思")

o = s:option(Value, "max_download_load", translate("同时进行下载文件的最大数量"))
o.placeholder = "1"
o.description = translate("不要太贪心, 当心被封号")

o = s:option(Value, "max_parallel", translate("最大并发连接数"))
o.placeholder = "8"

return m
