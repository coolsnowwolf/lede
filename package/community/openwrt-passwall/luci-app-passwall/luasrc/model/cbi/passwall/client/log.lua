local api = require "luci.model.cbi.passwall.api.api"
local appname = api.appname

f = SimpleForm(appname)
f.reset = false
f.submit = false
f:append(Template(appname .. "/log/log"))
return f
