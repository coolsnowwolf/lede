f = SimpleForm("serverchan")
f.reset = false
f.submit = false
f:append(Template("serverchan/serverchan_log"))
return f
