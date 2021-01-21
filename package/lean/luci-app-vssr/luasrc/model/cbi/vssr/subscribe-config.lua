local vssr = 'vssr'
local uci = luci.model.uci.cursor()
local server_table = {}

local gfwmode = 0
local gfw_count = 0
local ip_count = 0
local ad_count = 0
local server_count = 0

local sys = require 'luci.sys'

uci:foreach(
    'vssr',
    'servers',
    function(s)
        server_count = server_count + 1
    end
)

m = Map(vssr)

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
o.description = translate('You can manually add group names in front of the URL, splited by ,')

o = s:option(Value, 'filter_words', translate('Subscribe Filter Words'))
o.rmempty = true
o.description = translate('Filter Words splited by /')

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

return m
