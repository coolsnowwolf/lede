local vssr = 'vssr'
local uci = luci.model.uci.cursor()
local server_table = {}

local gfwmode = 0
local gfw_count = 0
local ip_count = 0
local ad_count = 0
local server_count = 0

if nixio.fs.access('/etc/dnsmasq.ssr/gfw_list.conf') then
    gfwmode = 1
end

local sys = require 'luci.sys'

if gfwmode == 1 then
    gfw_count = tonumber(sys.exec('cat /etc/dnsmasq.ssr/gfw_list.conf | wc -l')) / 2
    if nixio.fs.access('/etc/dnsmasq.ssr/ad.conf') then
        ad_count = tonumber(sys.exec('cat /etc/dnsmasq.ssr/ad.conf | wc -l'))
    end
end

if nixio.fs.access('/etc/china_ssr.txt') then
    ip_count = sys.exec('cat /etc/china_ssr.txt | wc -l')
end

uci:foreach(
    'vssr',
    'servers',
    function(s)
        server_count = server_count + 1
    end
)

m = Map(vssr)

-- [[ 服务器节点故障自动切换设置 ]]--

s = m:section(TypedSection, 'global', translate('Server failsafe auto swith settings'))
s.anonymous = true

o = s:option(Flag, 'monitor_enable', translate('Enable Process Deamon'))
o.rmempty = false

o = s:option(Flag, 'enable_switch', translate('Enable Auto Switch'))
o.rmempty = false

o = s:option(Value, 'switch_time', translate('Switch check cycly(second)'))
o.datatype = 'uinteger'
o:depends('enable_switch', '1')
o.default = 3600

o = s:option(Value, 'switch_timeout', translate('Check timout(second)'))
o.datatype = 'uinteger'
o:depends('enable_switch', '1')
o.default = 5

o = s:option(Value, 'switch_try_count', translate('Check Try Count'))
o.datatype = 'uinteger'
o:depends('enable_switch', '1')
o.default = 3

-- [[ 节点订阅 ]]--

s = m:section(TypedSection, 'server_subscribe', translate('Servers subscription and manage'))
s.anonymous = true

o = s:option(Flag, 'auto_update', translate('Auto Update'))
o.rmempty = false
o.description = translate('Auto Update Server subscription, GFW list and CHN route')

o = s:option(ListValue, 'auto_update_time', translate('Update time (every day)'))
for t = 0, 23 do
    o:value(t, t .. ':00')
end
o.default = 2
o.rmempty = false

o = s:option(DynamicList, 'subscribe_url', translate('Subscribe URL'))
o.rmempty = true

o = s:option(Flag, 'proxy', translate('Through proxy update'))
o.rmempty = false
o.description = translate('Through proxy update list, Not Recommended ')

o = s:option(DummyValue, '', '')
o.rawhtml = true
o.template = 'vssr/update_subscribe'

o = s:option(Button, 'delete', translate('Delete All Subscribe Severs'))
o.inputstyle = 'reset'
o.description = string.format(translate('Server Count') .. ': %d', server_count)
o.write = function()
    uci:delete_all(
        'vssr',
        'servers',
        function(s)
            if s.hashkey or s.isSubscribe then
                return true
            else
                return false
            end
        end
    )
    uci:save('vssr')
    uci:commit('vssr')
    luci.sys.exec('/etc/init.d/vssr restart')
    luci.http.redirect(luci.dispatcher.build_url('admin', 'services', 'vssr', 'servers'))
    return
end

-- [[ adblock ]]--
s = m:section(TypedSection, 'global', translate('adblock settings'))
s.anonymous = true

o = s:option(Flag, 'adblock', translate('Enable adblock'))
o.rmempty = false

-- [[ 更新设置 ]]--

s = m:section(TypedSection, 'socks5_proxy', translate('Update Setting'))
s.anonymous = true

o = s:option(Button, 'gfw_data', translate('GFW List Data'))
o.rawhtml = true
o.template = 'vssr/refresh'
o.value = tostring(math.ceil(gfw_count)) .. ' ' .. translate('Records')

o = s:option(Button, 'ip_data', translate('China IP Data'))
o.rawhtml = true
o.template = 'vssr/refresh'
o.value = ip_count .. ' ' .. translate('Records')

if uci:get_first('vssr', 'global', 'adblock', '') == '1' then
    o = s:option(Button, 'ad_data', translate('Advertising Data'))
    o.rawhtml = true
    o.template = 'vssr/refresh'
    o.value = ad_count .. ' ' .. translate('Records')
end

return m
