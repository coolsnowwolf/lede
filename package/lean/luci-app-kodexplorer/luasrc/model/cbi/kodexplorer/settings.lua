m = Map("kodexplorer")
m.title = translate("KodExplorer")
m.description = translate("KodExplorer is a fast and efficient private cloud and online document management system that provides secure, controllable, easy-to-use and highly customizable private cloud products for personal websites, enterprise private cloud deployment, network storage, online document management, and online office. With Windows style interface and operation habits, it can be used quickly without adaptation. It supports online preview of hundreds of common file formats and is extensible and easy to customize.")

m:section(SimpleSection).template = "kodexplorer/kodexplorer_status"

s = m:section(TypedSection, "global", translate("Global Settings"))
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enable", translate("Enable"))
o.rmempty = false

o = s:option(Value, "port", translate("Nginx Port"))
o.datatype = "port"
o.default = 8081
o.rmempty = false

o = s:option(Value, "memory_limit", translate("Maximum memory usage"))
o.description = translate("If your device has a lot of memory, you can increase it.")
o.default = "8M"
o.rmempty = false

o = s:option(Value, "post_max_size", translate("Maximum POST capacity"))
o.description = translate("This value cannot be greater than the maximum memory usage")
o.default = "12M"
o.rmempty = false

o = s:option(Value, "upload_max_filesize", translate("Maximum memory usage for uploading files"))
o.description = translate("This value cannot be greater than the POST maximum capacity")
o.default = "12M"
o.rmempty = false

o = s:option(Value, "storage_device_path", translate("Storage device path"))
o.description = translate("It is recommended to insert a usb flash drive or hard disk and enter the path. For example, /mnt/sda1/")
o.default = "/mnt/sda1/"
o.rmempty = false

o = s:option(Value, "project_directory", translate("Project directory"))
o.description = translate("It is recommended to insert a usb flash drive or hard disk and enter the path. For example, /mnt/sda1/kodexplorer")
o.default = "/mnt/sda1/kodexplorer"
o.rmempty = false

s:section(SimpleSection).template = "kodexplorer/kodexplorer_version"

o = s:option(Button, "_download", translate("Manually update"))
o.description = translate("Make sure you have enough space.<br /><font style='color:red'>Be sure to fill out the device path and store path for the first run, and then save the application. Then manually download, otherwise can not use!</font>")
o.template = "kodexplorer/kodexplorer_download"
o.inputstyle = "apply"
o.btnclick = "downloadClick(this);"
o.id = "download_btn"

return m
