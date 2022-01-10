m = Map("kodexplorer", translate("KodExplorer"), translate("KodExplorer is a fast and efficient private cloud and online document management system that provides secure, controllable, easy-to-use and highly customizable private cloud products for personal websites, enterprise private cloud deployment, network storage, online document management, and online office. With Windows style interface and operation habits, it can be used quickly without adaptation. It supports online preview of hundreds of common file formats and is extensible and easy to customize."))
m:append(Template("kodexplorer/status"))

s = m:section(TypedSection, "global")
s.anonymous = true
s.addremove = false

s:tab("global",  translate("Global Settings"))
s:tab("template", translate("Edit Template"))

php_fpm = s:taboption("template", Value, "_php_fpm",
	translatef("Edit the template that is used for generating the %s configuration.", "php-fpm"), 
    translatef("This is the content of the file '%s'", "/etc/kodexplorer/php-fpm.conf.template") ..
    translatef("Values enclosed by pipe symbols ('|') should not be changed. They get their values from the '%s' tab.", translate("Global Settings")))
php_fpm.template = "cbi/tvalue"
php_fpm.rows = 10

function php_fpm.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/kodexplorer/php-fpm.conf.template")
end

function php_fpm.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("/etc/kodexplorer/php-fpm.conf.template", value)
end

php = s:taboption("template", Value, "_php",
	translatef("Edit the template that is used for generating the %s configuration.", "php"), 
    translatef("This is the content of the file '%s'", "/etc/kodexplorer/php.ini.template") ..
    translatef("Values enclosed by pipe symbols ('|') should not be changed. They get their values from the '%s' tab.", translate("Global Settings")))
php.template = "cbi/tvalue"
php.rows = 10

function php.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/kodexplorer/php.ini.template")
end

function php.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("/etc/kodexplorer/php.ini.template", value)
end

nginx = s:taboption("template", Value, "_nginx",
	translatef("Edit the template that is used for generating the %s configuration.", "nginx"), 
    translatef("This is the content of the file '%s'", "/etc/kodexplorer/nginx.conf.template") ..
    translatef("Values enclosed by pipe symbols ('|') should not be changed. They get their values from the '%s' tab.", translate("Global Settings")))
nginx.template = "cbi/tvalue"
nginx.rows = 10

function nginx.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/kodexplorer/nginx.conf.template")
end

function nginx.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("/etc/kodexplorer/nginx.conf.template", value)
end

o = s:taboption("global", Flag, "enable", translate("Enable"))
o.rmempty = false

o = s:taboption("global", Flag, "ipv6", translate("Listen IPv6"))
o.rmempty = false

o = s:taboption("global", Value, "port", translate("Nginx Port"))
o.datatype = "port"
o.default = 8081
o.rmempty = false

o = s:taboption("global", Flag, "https", translate("HTTPS"))
o.rmempty = false

o = s:taboption("global", FileUpload, "certificate", translate("certificate"))
o:depends("https", 1)

o = s:taboption("global", FileUpload, "key", translate("key"))
o:depends("https", 1)

o = s:taboption("global", Value, "memory_limit", translate("Maximum memory usage"), translate("If your device has a lot of memory, you can increase it."))
o.default = "32M"
o.rmempty = false

o = s:taboption("global", Value, "upload_max_filesize", translate("Maximum memory usage for uploading files"))
o.default = "32M"
o.rmempty = false

o = s:taboption("global", DynamicList, "open_basedir", translate("Accessible directory"))
o.rmempty = false

o = s:taboption("global", Value, "project_directory", translate("Project directory"), translate("It is recommended to insert a usb flash drive or hard disk and enter the path. For example, /mnt/sda1/kodexplorer"))
o.default = "/mnt/sda1/kodexplorer"
o.rmempty = false

o = s:taboption("global", Button, "_update")
o.template = "kodexplorer/version"
return m
