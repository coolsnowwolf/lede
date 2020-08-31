local vssr = 'vssr'
local uci = luci.model.uci.cursor()
local server_table = {}

local sys = require 'luci.sys'

m = Map(vssr)

-- [[ SOCKS5 Proxy ]]--
if nixio.fs.access('/usr/bin/v2ray/v2ray') then
    s = m:section(TypedSection, 'socks5_proxy', translate('V2ray SOCKS5 Proxy'))
    s.anonymous = true

    o = s:option(Flag, 'enable_server', translate('Enable Servers'))
    o.rmempty = false

    o = s:option(Flag, 'enable_auth', translate('Enable Auth'))
    o.rmempty = false

    o = s:option(Value, 'Socks_user', translate('Socks user'))
    o.default = 'user'
    o.rmempty = true
    o:depends('enable_auth', '1')

    o = s:option(Value, 'Socks_pass', translate('Socks pass'))
    o.default = 'password'
    o.password = true
    o.rmempty = true
    o:depends('enable_auth', '1')

    o = s:option(Value, 'local_port', translate('Local Port'))
    o.datatype = 'port'
    o.default = 1080
    o.rmempty = false
end

return m
