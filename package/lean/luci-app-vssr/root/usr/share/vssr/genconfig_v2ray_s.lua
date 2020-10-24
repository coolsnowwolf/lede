local ucursor = require 'luci.model.uci'.cursor()
local name = 'vssr'
local json = require 'luci.jsonc'
local proto = 'socks'
local auth_type = ucursor:get_first(name, 'socks5_proxy', 'enable_auth')
local local_port = ucursor:get_first(name, 'socks5_proxy', 'local_port')
local Socks_user = ucursor:get_first(name, 'socks5_proxy', 'Socks_user')
local Socks_pass = ucursor:get_first(name, 'socks5_proxy', 'Socks_pass')

local v2ray = {
    log = {
        -- error = "/var/log/v2ray.log",
        loglevel = 'warning'
    },
    -- 传入连接
    inbound = {
        port = local_port,
        protocol = proto,
        settings = {
            auth = (auth_type == '1') and 'password' or 'noauth',
            accounts = (auth_type == '1') and {{user = (auth_type == '1') and Socks_user, pass = Socks_pass}} or nil
        }
    },
    -- 传出连接
    outbound = {protocol = 'freedom'},
    -- 额外传出连接
    outboundDetour = {{protocol = 'blackhole', tag = 'blocked'}}
}
print(json.stringify(v2ray, 1))
