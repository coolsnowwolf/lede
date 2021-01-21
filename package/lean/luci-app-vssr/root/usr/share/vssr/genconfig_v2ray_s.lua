local ucursor = require 'luci.model.uci'.cursor()
local name = 'vssr'
local json = require 'luci.jsonc'
local proto = 'socks'
local socks_switch = ucursor:get_first(name, 'socks5_proxy', 'enable_server')
local auth_type = ucursor:get_first(name, 'socks5_proxy', 'enable_auth')
local local_port = ucursor:get_first(name, 'socks5_proxy', 'local_port')
local Socks_user = ucursor:get_first(name, 'socks5_proxy', 'Socks_user')
local Socks_pass = ucursor:get_first(name, 'socks5_proxy', 'Socks_pass')

local http_switch = ucursor:get_first(name, 'socks5_proxy', 'enable_server')
local auth_typeh = ucursor:get_first(name, 'http_proxy', 'enable_auth')
local local_porth = ucursor:get_first(name, 'http_proxy', 'local_port')
local http_user = ucursor:get_first(name, 'http_proxy', 'http_user')
local http_pass = ucursor:get_first(name, 'http_proxy', 'http_pass')

function gen_inbound(sw, auth_type, local_port, user, pass, proto)
    local bound = {}
    if sw == 0 then
        bound = nil
    else
        bound = {
            port = local_port,
            protocol = proto,
            settings = {
                auth = (auth_type == '1') and 'password' or 'noauth',
                accounts = (auth_type == '1') and {{user = (auth_type == '1') and user, pass = pass}} or nil
            }
        }
    end
    return bound
end

inbounds_table = {}

table.insert(inbounds_table, gen_inbound(socks_switch, auth_type, local_port, Socks_user, Socks_pass, 'socks'))
table.insert(inbounds_table, gen_inbound(http_switch, auth_typeh, local_porth, http_user, http_pass, 'http'))

local v2ray = {
    log = {
        -- error = "/var/log/v2ray.log",
        loglevel = 'warning'
    },
    -- 传入连接
    inbounds = inbounds_table,
    -- 传出连接
    outbound = {protocol = 'freedom'},
    -- 额外传出连接
    outboundDetour = {{protocol = 'blackhole', tag = 'blocked'}}
}
print(json.stringify(v2ray, 1))
