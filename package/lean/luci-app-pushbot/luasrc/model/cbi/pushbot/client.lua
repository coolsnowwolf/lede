f = SimpleForm("pushbot")
luci.sys.call("/usr/bin/pushbot/pushbot client")
f.reset = false
f.submit = false
f:append(Template("pushbot/client"))
return f
