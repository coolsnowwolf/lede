local fs = require 'nixio.fs'
local conffile = '/tmp/ssrpro.log'

f = SimpleForm('logview')

t = f:field(TextValue, 'conf')
t.rmempty = true
t.rows = 20
function t.cfgvalue()
    luci.sys.exec("[ -f /tmp/vssr.log ] && sed '1!G;h;$!d' /tmp/vssr.log > /tmp/ssrpro.log")
    return fs.readfile(conffile) or ''
end
t.readonly = 'readonly'

return f
