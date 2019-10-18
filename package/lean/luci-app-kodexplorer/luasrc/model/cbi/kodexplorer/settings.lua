m = Map("kodexplorer",translate("KodExplorer"),translate("KodExplorer是一款快捷高效的私有云和在线文档管理系统，为个人网站、企业私有云部署、网络存储、在线文档管理、在线办公等提供安全可控，简便易用、可高度定制的私有云产品。采用windows风格界面、操作习惯，无需适应即可快速上手，支持几百种常用文件格式的在线预览，可扩展易定制。"))
m:append(Template("kodexplorer/status"))

s = m:section(TypedSection,"global",translate("Global Setting"))
s.anonymous = true
s.addremove = false

o = s:option(Flag,"enable",translate("Enable"))
o.rmempty = false

o = s:option(Value, "port", translate("Nginx监听端口"))
o.datatype="port"
o.default=81
o.rmempty = false

o = s:option(Value, "memory_limit", translate("内存最大使用"), translate("如果你的设备内存较大的话，可以适当增加。"))
o.default="8M"
o.rmempty = false

o = s:option(Value, "post_max_size", translate("POST最大容量"), translate("该值不能大于 内存最大使用"))
o.default="12M"
o.rmempty = false

o = s:option(Value, "upload_max_filesize", translate("上传文件最大使用内存"), translate("该值不能大于 POST最大容量"))
o.default="12M"
o.rmempty = false

o = s:option(Value, "storage_device_path", translate("存储设备路径"), translate("建议插入U盘或硬盘，然后输入路径。例如：/mnt/sda1/"))
o.default="/mnt/sda1/"
o.rmempty = false

o = s:option(Value, "project_directory", translate("项目存放目录"), translate("建议插入U盘或硬盘，然后输入路径。例如：/mnt/sda1/kodexplorer"))
o.default="/mnt/sda1/kodexplorer"
o.rmempty = false

o = s:option(Button, "_download", translate("手动下载"),
	translate("请确保具有足够的空间。<br /><font style='color:red'>第一次运行务必填好设备路径和存放路径，然后保存应用。再手动下载，否则无法使用！</font>"))
o.template = "kodexplorer/download"
o.inputstyle = "apply"
o.btnclick = "downloadClick(this);"
o.id="download_btn"

return m
