f = SimpleForm("serverchan")
luci.sys.call("/usr/bin/serverchan/serverchan client")
f.reset = false
f.submit = false
f:append(Template("serverchan/serverchan_client"))
return f
