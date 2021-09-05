-- Copyright (C) 2017 yushi studio <ywb94@qq.com> github.com/ywb94
-- Copyright (C) 2018 lean <coolsnowwolf@gmail.com> github.com/coolsnowwolf
-- Licensed to the public under the GNU General Public License v3.
local m, s, sec, o, kcp_enable
local vssr = 'vssr'
local gfwmode = 0


if nixio.fs.access('/etc/vssr/gfw_list.conf') then 
    gfwmode = 1
end

local uci = luci.model.uci.cursor()

m = Map(vssr)

m:section(SimpleSection).template = 'vssr/status_top'

local server_table = {}
local tw_table = {}
local tvb_table = {}


local server_table = {}
uci:foreach(
    'vssr',
    'servers',
    function(s)
        if s.type ~= nil then
            s['name'] = s['.name']
            local alias = (s.alias ~= nil) and s.alias or "未命名节点"
            s['gname'] = '[%s]:%s' % {string.upper(s.type), alias}
            table.insert(server_table, s)
        end
    end
)
function my_sort(a,b)
    if(a.alias ~= nil and b.alias ~= nil) then
        return  a.alias < b.alias
    end
end
table.sort(server_table, my_sort)


local route_name = {
    'youtube_server',
    'tw_video_server',
    'netflix_server',
    'disney_server',
    'prime_server',
    'tvb_server',
    'custom_server'
}
local route_label = {
    'Youtube Proxy',
    'TaiWan Video Proxy',
    'Netflix Proxy',
    'Disney+ Proxy',
    'Prime Video Proxy',
    'TVB Video Proxy',
    'Custom Proxy'
}

-- [[ Global Setting ]]--
s = m:section(TypedSection, 'global', translate('Basic Settings [SS|SSR|Xray|Trojan]'))
s.anonymous = true

o = s:option(ListValue, 'global_server', translate('Main Server'))
o:value('nil', translate('Disable'))
for _, key in pairs(server_table) do
    o:value(key.name, key.gname)
end
o.default = 'nil'
o.rmempty = false

o = s:option(ListValue, 'udp_relay_server', translate('Game Mode UDP Server'))
o:value('', translate('Disable'))
o:value('same', translate('Same as Main Server'))
for _, key in pairs(server_table) do
    o:value(key.name, key.gname)
end

o = s:option(Flag, 'v2ray_flow', translate('Open v2ray route'))
o.rmempty = false
o.description = translate('When open v2ray routed,Apply may take more time.')

for i, v in pairs(route_name) do
    o = s:option(ListValue, v, translate(route_label[i]))
    o:value('nil', translate('Same as Main Server'))
    for _, key in pairs(server_table) do
        o:value(key.name, key.gname)
    end
    o:depends('v2ray_flow', '1')
    o.default = 'nil'
end

o = s:option(ListValue, 'threads', translate('Multi Threads Option'))
o:value('0', translate('Auto Threads'))
o:value('1', translate('1 Thread'))
o:value('2', translate('2 Threads'))
o:value('4', translate('4 Threads'))
o:value('8', translate('8 Threads'))
o.default = '0'
o.rmempty = false

o = s:option(ListValue, 'run_mode', translate('Running Mode'))
o:value('gfw', translate('GFW List Mode'))
o:value('router', translate('IP Route Mode'))
o:value('all', translate('Global Mode'))
o:value('oversea', translate('Oversea Mode'))
o.default = 'router'
o = s:option(ListValue, 'dports', translate('Proxy Ports'))
o:value('1', translate('All Ports'))
o:value('2', translate('Only Common Ports'))
o.default = 1

o = s:option(ListValue, 'pdnsd_enable', translate('Resolve Dns Mode'))
o:value('1', translate('Use Pdnsd tcp query and cache'))
o:value('0', translate('Use Local DNS Service listen port 5335'))
o.default = 1

o = s:option(Value, 'tunnel_forward', translate('Anti-pollution DNS Server'))
o:value('8.8.4.4:53', translate('Google Public DNS (8.8.4.4)'))
o:value('8.8.8.8:53', translate('Google Public DNS (8.8.8.8)'))
o:value('208.67.222.222:53', translate('OpenDNS (208.67.222.222)'))
o:value('208.67.220.220:53', translate('OpenDNS (208.67.220.220)'))
o:value('209.244.0.3:53', translate('Level 3 Public DNS (209.244.0.3)'))
o:value('209.244.0.4:53', translate('Level 3 Public DNS (209.244.0.4)'))
o:value('4.2.2.1:53', translate('Level 3 Public DNS (4.2.2.1)'))
o:value('4.2.2.2:53', translate('Level 3 Public DNS (4.2.2.2)'))
o:value('4.2.2.3:53', translate('Level 3 Public DNS (4.2.2.3)'))
o:value('4.2.2.4:53', translate('Level 3 Public DNS (4.2.2.4)'))
o:value('1.1.1.1:53', translate('Cloudflare DNS (1.1.1.1)'))
o:value('114.114.114.114:53', translate('Oversea Mode DNS-1 (114.114.114.114)'))
o:value('114.114.115.115:53', translate('Oversea Mode DNS-2 (114.114.115.115)'))
o:depends('pdnsd_enable', '1')

m:section(SimpleSection).template = 'vssr/status_bottom'
return m
