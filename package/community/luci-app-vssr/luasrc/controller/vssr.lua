-- Copyright (C) 2018 jerrykuku <jerrykuku@qq.com>
-- Licensed to the public under the GNU General Public License v3.
module('luci.controller.vssr', package.seeall)

function index()
    if not nixio.fs.access('/etc/config/vssr') then
        return
    end

    if nixio.fs.access('/usr/bin/ssr-redir') then
        entry({'admin', 'services', 'vssr'}, alias('admin', 'services', 'vssr', 'client'), _('Hello World'), 0).dependent = true -- 首页
        entry({'admin', 'services', 'vssr', 'client'}, cbi('vssr/client'), _('SSR Client'), 10).leaf = true -- 基本设置
        entry({'admin', 'services', 'vssr', 'servers'}, cbi('vssr/servers'), _('Severs Nodes'), 11).leaf = true -- 服务器节点
        entry({'admin', 'services', 'vssr', 'servers'}, arcombine(cbi('vssr/servers'), cbi('vssr/client-config')), _('Severs Nodes'), 11).leaf = true -- 编辑节点
        entry({'admin', 'services', 'vssr', 'subscribe_config'}, cbi('vssr/subscribe-config', {hideapplybtn = true, hidesavebtn = true, hideresetbtn = true}), _('Subscribe'), 12).leaf = true -- 订阅设置
        entry({'admin', 'services', 'vssr', 'control'}, cbi('vssr/control'), _('Access Control'), 13).leaf = true -- 访问控制
        entry({'admin', 'services', 'vssr', 'router'}, cbi('vssr/router'), _('Router Config'), 14).leaf = true -- 访问控制
        if nixio.fs.access('/usr/bin/xray') then
            entry({'admin', 'services', 'vssr', 'socks5'}, cbi('vssr/socks5'), _('Local Proxy'), 15).leaf = true -- Socks5代理
        end
        entry({'admin', 'services', 'vssr', 'advanced'}, cbi('vssr/advanced'), _('Advanced Settings'), 16).leaf = true -- 高级设置
    elseif nixio.fs.access('/usr/bin/ssr-server') then
        entry({'admin', 'services', 'vssr'}, alias('admin', 'services', 'vssr', 'server'), _('vssr'), 10).dependent = true
    else
        return
    end
    if nixio.fs.access('/usr/bin/ssr-server') then
        entry({'admin', 'services', 'vssr', 'server'}, arcombine(cbi('vssr/server'), cbi('vssr/server-config')), _('SSR Server'), 20).leaf = true -- 服务端
    end

    entry({'admin', 'services', 'vssr', 'log'}, cbi('vssr/log'), _('Log'), 30).leaf = true -- 日志
    --entry({'admin', 'services', 'vssr', 'licence'}, template('vssr/licence'), _('Licence'), 40).leaf = true

    entry({'admin', 'services', 'vssr', 'refresh'}, call('refresh_data')) -- 更新白名单和GFWLIST
    entry({'admin', 'services', 'vssr', 'checkport'}, call('check_port')) -- 检测单个端口并返回Ping
    entry({'admin', 'services', 'vssr', 'run'}, call('act_status')) -- 检测全局服务器状态
    entry({'admin', 'services', 'vssr', 'change'}, call('change_node')) -- 切换节点
    entry({'admin', 'services', 'vssr', 'allserver'}, call('get_servers')) -- 获取所有节点Json
    entry({'admin', 'services', 'vssr', 'subscribe'}, call('get_subscribe')) -- 执行订阅
    entry({'admin', 'services', 'vssr', 'flag'}, call('get_flag')) -- 获取节点国旗 iso code
    entry({'admin', 'services', 'vssr', 'ip'}, call('check_ip')) -- 获取ip情况
    entry({'admin', 'services', 'vssr', 'switch'}, call('switch')) -- 设置节点为自动切换
    entry({'admin', 'services', 'vssr', 'delnode'}, call('del_node')) -- 删除某个节点
end

-- 执行订阅
function get_subscribe()
    local cjson = require 'luci.jsonc'
    local e = {}
    local name = 'vssr'
    local uci = luci.model.uci.cursor()
    local auto_update = luci.http.formvalue('auto_update')
    local auto_update_time = luci.http.formvalue('auto_update_time')
    local proxy = luci.http.formvalue('proxy')
    local subscribe_url = luci.http.formvalue('subscribe_url')
    local filter_words = luci.http.formvalue('filter_words')
    if subscribe_url ~= '[]' then
        uci:delete(name, '@server_subscribe[0]', subscribe_url)
        uci:set(name, '@server_subscribe[0]', 'auto_update', auto_update)
        uci:set(name, '@server_subscribe[0]', 'auto_update_time', auto_update_time)
        uci:set(name, '@server_subscribe[0]', 'proxy', proxy)
        uci:set(name, '@server_subscribe[0]', 'filter_words', filter_words)
        uci:set_list(name, '@server_subscribe[0]', 'subscribe_url', cjson.parse(subscribe_url))
        uci:commit(name)
        luci.sys.exec('/usr/bin/lua /usr/share/vssr/subscribe.lua >/www/check_update.htm 2>/dev/null &')
        e.error = 0
    else
        e.error = 1
    end
    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

-- 获取所有节点
function get_servers()
    local uci = luci.model.uci.cursor()
    local server_table = {}
    uci:foreach(
        'vssr',
        'servers',
        function(s)
            local e = {}
            e['name'] = s['.name']
            table.insert(server_table, e)
        end
    )
    luci.http.prepare_content('application/json')
    luci.http.write_json(server_table)
end

-- 删除指定节点
function del_node()
    local e = {}
    local uci = luci.model.uci.cursor()
    local node = luci.http.formvalue('node')
    e.status = false
    e.node = node
    if node ~= '' then
        uci:delete('vssr', node)
        uci:save('vssr')
        uci:commit('vssr')
        e.status = true
    end
    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

-- 切换节点
function change_node()
    local sockets = require 'socket'
    local e = {}
    local uci = luci.model.uci.cursor()
    local sid = luci.http.formvalue('set')
    local server = luci.http.formvalue('server')
    e.status = false
    e.sid = sid
    if sid ~= '' and server ~= '' then
        uci:set('vssr', '@global[0]', server .. '_server', sid)
        if (server ~= 'global' and server ~= 'udp_relay') then
            uci:set('vssr', '@global[0]', 'v2ray_flow', '1')
        end
        uci:commit('vssr')
        luci.sys.call('/etc/init.d/vssr restart >/www/restartlog.htm 2>&1')
        e.status = true
    end
    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

-- 设置节点为自动切换
function switch()
    local e = {}
    local uci = luci.model.uci.cursor()
    local sid = luci.http.formvalue('node')
    local isSwitch = uci:get('vssr', sid, 'switch_enable')
    local toSwitch = (isSwitch == '1') and '0' or '1'
    uci:set('vssr', sid, 'switch_enable', toSwitch)
    uci:commit('vssr')
    if isSwitch == '1' then
        e.switch = false
    else
        e.switch = true
    end
    e.status = true
    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

-- 检测全局服务器状态
function act_status()
    math.randomseed(os.time())
    local e = {}
    -- 全局服务器
    e.global = luci.sys.call('busybox ps -w | grep vssr_t | grep -v grep >/dev/null') == 0
    -- 检测PDNSD状态
    e.pdnsd = luci.sys.call('pidof pdnsd >/dev/null') == 0
    -- 检测游戏模式状态
    e.game = luci.sys.call('busybox ps -w | grep vssr_u | grep -v grep >/dev/null') == 0
    -- 检测Socks5
    e.socks5 = luci.sys.call('busybox ps -w | grep vssr_s | grep -v grep >/dev/null') == 0
    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

-- 检测单个节点状态并返回连接速度
function check_port()
    local sockets = require 'socket'
    local vssr = require 'vssrutil'
    local set = luci.http.formvalue('host')
    local port = luci.http.formvalue('port')
    local retstring = ''
    local t0 = sockets.gettime()
    ret = vssr.check_site(set, port)
    local t1 = sockets.gettime()
    retstring = tostring(ret) == 'true' and '1' or '0'
    local tt = t1 - t0
    luci.http.prepare_content('application/json')
    luci.http.write_json({ret = retstring, used = math.floor(tt * 1000 + 0.5)})
end

function get_iso(ip)
    local mm = require 'maxminddb'
    local db = mm.open('/usr/share/vssr/GeoLite2-Country.mmdb')
    local res = db:lookup(ip)
    return string.lower(res:get('country', 'iso_code'))
end

function get_cname(ip)
    local mm = require 'maxminddb'
    local db = mm.open('/usr/share/vssr/GeoLite2-Country.mmdb')
    local res = db:lookup(ip)
    return string.lower(res:get('country', 'names', 'zh-CN'))
end

-- 获取当前代理状态 与节点ip
function check_ip()
    local e = {}
    local d = {}
    local vssr = require 'vssrutil'
    local port = 80
    local ip = vssr.wget('http://api.ipify.org/')
    d.flag = 'un'
    d.country = 'Unknown'
    if (ip ~= '') then
        local status, code = pcall(get_iso, ip)
        if (status) then
            d.flag = code
        end
        local status1, country = pcall(get_cname, ip)
        if (status1) then
            d.country = country
        end
    end
    e.outboard = ip
    e.outboardip = d
    e.baidu = vssr.check_site('www.baidu.com', port)
    e.taobao = vssr.check_site('www.taobao.com', port)
    e.google = vssr.check_site('www.google.com', port)
    e.youtube = vssr.check_site('www.youtube.com', port)
    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

-- 获取节点国旗 iso code
function get_flag()
    local e = {}
    local vssr = require 'vssrutil'
    local host = luci.http.formvalue('host')
    local remark = luci.http.formvalue('remark')
    e.host = host
    e.flag = vssr.get_flag(remark, host)
    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

-- 刷新检测文件

function refresh_data()
    local set = luci.http.formvalue('set')
    local icount = 0

    if set == 'gfw_data' then
        refresh_cmd = 'wget-ssl --no-check-certificate https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt -O /tmp/gfw.b64'
        sret = luci.sys.call(refresh_cmd .. ' 2>/dev/null')
        if sret == 0 then
            luci.sys.call('/usr/bin/vssr-gfw')
            icount = luci.sys.exec('cat /tmp/gfwnew.txt | wc -l')
            if tonumber(icount) > 1000 then
                oldcount = luci.sys.exec('cat /etc/vssr/gfw_list.conf | wc -l')
                if tonumber(icount) ~= tonumber(oldcount) then
                    luci.sys.exec('cp -f /tmp/gfwnew.txt /etc/vssr/gfw_list.conf')
                    retstring = tostring(math.ceil(tonumber(icount) / 2))
                else
                    retstring = '0'
                end
            else
                retstring = '-1'
            end
            luci.sys.exec('rm -f /tmp/gfwnew.txt ')
        else
            retstring = '-1'
        end
    elseif set == 'ip_data' then
        refresh_cmd = "wget-ssl -O- 'https://ispip.clang.cn/all_cn.txt' > /tmp/china_ssr.txt 2>/dev/null"
        sret = luci.sys.call(refresh_cmd)
        icount = luci.sys.exec('cat /tmp/china_ssr.txt | wc -l')
        if sret == 0 and tonumber(icount) > 1000 then
            oldcount = luci.sys.exec('cat /etc/vssr/china_ssr.txt | wc -l')
            if tonumber(icount) ~= tonumber(oldcount) then
                luci.sys.exec('cp -f /tmp/china_ssr.txt /etc/vssr/china_ssr.txt')
                retstring = tostring(tonumber(icount))
            else
                retstring = '0'
            end
        else
            retstring = '-1'
        end
        luci.sys.exec('rm -f /tmp/china_ssr.txt ')
    else
        local need_process = 0
        refresh_cmd = 'wget-ssl --no-check-certificate -O - https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt > /tmp/adnew.conf'
        need_process = 1
        sret = luci.sys.call(refresh_cmd .. ' 2>/dev/null')
        if sret == 0 then
            if need_process == 1 then
                luci.sys.call('/usr/bin/vssr-ad')
            end
            icount = luci.sys.exec('cat /tmp/ad.conf | wc -l')
            if tonumber(icount) > 1000 then
                if nixio.fs.access('/etc/vssr/ad.conf') then
                    oldcount = luci.sys.exec('cat /etc/vssr/ad.conf | wc -l')
                else
                    oldcount = 0
                end
                if tonumber(icount) ~= tonumber(oldcount) then
                    luci.sys.exec('cp -f /tmp/ad.conf /etc/vssr/ad.conf')
                    retstring = tostring(math.ceil(tonumber(icount)))
                    if oldcount == 0 then
                        luci.sys.call('/etc/init.d/dnsmasq restart')
                    end
                else
                    retstring = '0'
                end
            else
                retstring = '-1'
            end
            luci.sys.exec('rm -f /tmp/ad.conf')
        else
            retstring = '-1'
        end
    end
    luci.http.prepare_content('application/json')
    luci.http.write_json({ret = retstring, retcount = icount})
end
