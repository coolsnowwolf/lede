-- Copyright (C) 2020 jerrykuku <jerrykuku@gmail.com>
-- Licensed to the public under the GNU General Public License v3.
module('luci.controller.jd-dailybonus', package.seeall)
package.path = package.path .. ';/usr/share/jd-dailybonus/?.lua'
function index()
    if not nixio.fs.access('/etc/config/jd-dailybonus') then
        return
    end

    entry({'admin', 'services', 'jd-dailybonus'}, alias('admin', 'services', 'jd-dailybonus', 'client'), _('京东签到服务'), 10).dependent = true -- 首页
    entry({'admin', 'services', 'jd-dailybonus', 'client'}, cbi('jd-dailybonus/client', {hidesavebtn = true, hideresetbtn = true}), _('客户端'), 10).leaf = true -- 基本设置
    entry({'admin', 'services', 'jd-dailybonus', 'log'}, form('jd-dailybonus/log'), _('日志'), 30).leaf = true -- 日志页面
    entry({'admin', 'services', 'jd-dailybonus', 'script'}, form('jd-dailybonus/script'), _('脚本查看'), 20).leaf = true -- 直接配置脚本
    entry({'admin', 'services', 'jd-dailybonus', 'run'}, call('run')) -- 执行程序
    entry({'admin', 'services', 'jd-dailybonus', 'update'}, call('update')) -- 执行更新
    entry({'admin', 'services', 'jd-dailybonus', 'check_update'}, call('check_update')) -- 检查更新
    entry({'admin', 'services', 'jd-dailybonus', 'qrcode'}, call('qrcode')) -- 获取二维码
    entry({'admin', 'services', 'jd-dailybonus', 'check_login'}, call('check_login')) -- 获取二维码
end

-- 写入配置
function write_json(file, content)
    local json = require 'luci.jsonc'
    local f = assert(io.open(file, 'w'))
    f:write(json.stringify(content, 1))
    f:close()
end
-- 执行程序
function run()
    local e = {}
    local uci = luci.model.uci.cursor()
    local data = luci.http.formvalue()

    data.auto_run = data.auto_run ~= nil and data.auto_run or 0
    data.auto_update = data.auto_update ~= nil and data.auto_update or 0
    uci:tset('jd-dailybonus', '@global[0]', data)
    uci:commit('jd-dailybonus')
    local json_data = {
        CookieJD = data.cookie,
        CookieJD2 = data.cookie2:len() == 0 and nil or data.cookie2,
        JD_DailyBonusDelay = data.stop,
        JD_DailyBonusTimeOut = data.out
    }
    write_json('/root/CookieSet.json', json_data)
    write_json('/www/CookieSet.json', json_data)
    luci.sys.call('/usr/share/jd-dailybonus/newapp.sh -r')
    luci.sys.call('/usr/share/jd-dailybonus/newapp.sh -a')
    e.error = 0

    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

--检查更新
function check_update()
    local jd = 'jd-dailybonus'
    local e = {}
    local new_version = luci.sys.exec('/usr/share/jd-dailybonus/newapp.sh -n')
    e.new_version = new_version
    e.error = 0
    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

--执行更新
function update()
    local jd = 'jd-dailybonus'
    local e = {}
    local uci = luci.model.uci.cursor()
    local version = luci.http.formvalue('version')
    --下载脚本
    local code = luci.sys.exec('/usr/share/jd-dailybonus/newapp.sh -u')
    e.error = code
    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

--获取二维码
function qrcode()
    local jd_cookie = require 'jd_cookie'
    local e = {}
    e.error = 0
    e.data = jd_cookie.get_qrcode()
    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

--检测登录
function check_login()
    local jd_cookie = require 'jd_cookie'
    local uci = luci.model.uci.cursor()
    local data = luci.http.formvalue()
    local id = data.id
    local e = jd_cookie.checkLogin(data.check_url, data.cookies)
    if e.error == 0 then
        local cookieStr = 'pt_key=' .. e.cookie.pt_key .. ';pt_pin=' .. e.cookie.pt_pin .. ';'
        uci:set('jd-dailybonus', '@global[0]', id, cookieStr)
        local json_data = {
            CookieJD = uci:get('jd-dailybonus', '@global[0]', 'cookie'),
            CookieJD2 = uci:get('jd-dailybonus', '@global[0]', 'cookie2'),
            JD_DailyBonusDelay = uci:get('jd-dailybonus', '@global[0]', 'stop'),
            JD_DailyBonusTimeOut = uci:get('jd-dailybonus', '@global[0]', 'out')
        }
        write_json('/root/CookieSet.json', json_data)
        write_json('/www/CookieSet.json', json_data)
        uci:commit('jd-dailybonus')
        e.cstr = cookieStr
    end

    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end
