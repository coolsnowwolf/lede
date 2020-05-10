f = SimpleForm("passwall")
f.reset = false
f.submit = false
f:append(Template("passwall/log/log"))
return f
