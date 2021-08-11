f = SimpleForm("pushbot")
f.reset = false
f.submit = false
f:append(Template("pushbot/log"))
return f
