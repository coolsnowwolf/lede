--[[
Author: your name
Date: 2019-11-11 09:33:07
LastEditTime: 2021-01-14 18:50:06
LastEditors: Please set LastEditors
Description: In User Settings Edit
FilePath: \luci-app-vssr\luasrc\model\cbi\vssr\socks5.lua
--]]
local vssr = 'vssr'
local uci = luci.model.uci.cursor()
local server_table = {}

local sys = require 'luci.sys'

m = Map(vssr)

-- [[ SOCKS5 Proxy ]]--
if nixio.fs.access('/usr/bin/xray') or nixio.fs.access('/usr/bin/xray/xray') then
    s = m:section(TypedSection, 'socks5_proxy', translate('Xray SOCKS5 Proxy'))
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

-- [[ Http Proxy ]]--
if nixio.fs.access('/usr/bin/xray') or nixio.fs.access('/usr/bin/xray/xray') then
    s = m:section(TypedSection, 'http_proxy', translate('Xray HTTP Proxy'))
    s.anonymous = true

    o = s:option(Flag, 'enable_server', translate('Enable Servers'))
    o.rmempty = false

    o = s:option(Flag, 'enable_auth', translate('Enable Auth'))
    o.rmempty = false

    o = s:option(Value, 'http_user', translate('HTTP user'))
    o.default = 'user'
    o.rmempty = true
    o:depends('enable_auth', '1')

    o = s:option(Value, 'http_pass', translate('HTTP pass'))
    o.default = 'password'
    o.password = true
    o.rmempty = true
    o:depends('enable_auth', '1')

    o = s:option(Value, 'local_port', translate('Local Port'))
    o.datatype = 'port'
    o.default = 1088
    o.rmempty = false
end

return m
