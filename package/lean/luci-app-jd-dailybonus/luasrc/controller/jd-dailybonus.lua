-- Copyright (C) 2020 jerrykuku <jerrykuku@gmail.com>
-- Licensed to the public under the GNU General Public License v3.
module("luci.controller.jd-dailybonus", package.seeall)
function index() 
    if not nixio.fs.access("/etc/config/jd-dailybonus") then 
        return 
    end
    
    entry({"admin", "services", "jd-dailybonus"}, alias("admin", "services", "jd-dailybonus", "client"), _("JD-DailyBonus"), 10).dependent = true -- 首页
    entry({"admin", "services", "jd-dailybonus", "client"}, cbi("jd-dailybonus/client"),_("Client"), 10).leaf = true -- 基本设置
    entry({"admin", "services", "jd-dailybonus", "log"},form("jd-dailybonus/log"),_("Log"), 80).leaf = true -- 日志页面
    entry({"admin", "services", "jd-dailybonus", "run"}, call("run")) -- 执行程序
    entry({"admin", "services", "jd-dailybonus", "update"}, call("update")) -- 执行更新
    entry({"admin", "services", "jd-dailybonus", "check_update"}, call("check_update")) -- 检查更新
end

-- 更新脚本cookie
function mod_script(cookie,cookie2)
    local varb = "var Key = '".. cookie .."'"
    local cmd5 = "sed -i '18d' /usr/share/jd-dailybonus/JD_DailyBonus.js"
    local cmd6 = 'sed -i "17a '.. varb ..'" -i /usr/share/jd-dailybonus/JD_DailyBonus.js'
    local varb2 = "var DualKey = '".. cookie2 .."'"
    local cmd5_1 = "sed -i '20d' /usr/share/jd-dailybonus/JD_DailyBonus.js"
    local cmd6_1 = 'sed -i "19a '.. varb2 ..'" -i /usr/share/jd-dailybonus/JD_DailyBonus.js'
    luci.sys.call(cmd5)
    luci.sys.call(cmd6)
    luci.sys.call(cmd5_1)
    luci.sys.call(cmd6_1)
end

-- 执行程序

function run()
    local e = {}
    local uci = luci.model.uci.cursor()
    local cookie = luci.http.formvalue("cookies")
    local cookie2 = luci.http.formvalue("cookies2")
    local auto_update = luci.http.formvalue("auto_update")
    local auto_update_time = luci.http.formvalue("auto_update_time")

    if cookie ~= " " then
        local cmd1 = 'uci set jd-dailybonus.@global[0].auto_update="' .. auto_update .. '"'
        local cmd2 = 'uci set jd-dailybonus.@global[0].auto_update_time="' .. auto_update_time .. '"'
        local cmd3 = 'uci set jd-dailybonus.@global[0].cookie="' .. cookie .. '"'
        local cmd3_1 = 'uci set jd-dailybonus.@global[0].cookie2="' .. cookie2 .. '"'
        local cmd4 = 'uci commit jd-dailybonus'
        luci.sys.call(cmd1)
        luci.sys.call(cmd2)
        luci.sys.call(cmd3)
        luci.sys.call(cmd3_1)
        luci.sys.call(cmd4)
        mod_script(cookie,cookie2)
        local date = os.date("%Y-%m-%d %H:%M:%S", os.time())
        luci.sys.call("echo '".. date .."' >/www/JD_DailyBonus.htm 2>/dev/null")
        luci.sys.call("nohup node /usr/share/jd-dailybonus/JD_DailyBonus.js >>/www/JD_DailyBonus.htm 2>/dev/null &")
        e.error = 0
    else
        e.error = 1
    end

    luci.http.prepare_content("application/json")
    luci.http.write_json(e)

end

--检查更新
function check_update()
    local jd = "jd-dailybonus"
    local e = {}
    local uci = luci.model.uci.cursor()
    local current_version =  uci:get_first(jd, 'global', 'version', '')
    local new_version = luci.sys.exec("wget -q -O - http://tmnch.china-giant.cn/jd_verison.txt")
    e.current_version = current_version
    e.new_version = new_version
    e.need_update = false
    if tonumber(new_version) > tonumber(current_version) then
        e.need_update = true
    end
    e.error = 0
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end

--执行更新
function update()
    local jd = "jd-dailybonus"
    local e = {}
    local uci = luci.model.uci.cursor()
    local version = luci.http.formvalue("version")
    local cookie = uci:get_first(jd, 'global', 'cookie', '')
    local cookie2 = uci:get_first(jd, 'global', 'cookie2', '')
    --下载脚本
    luci.sys.call("wget --no-check-certificate https://raw.githubusercontent.com/NobyDa/Script/master/JD-DailyBonus/JD_DailyBonus.js -O /usr/share/jd-dailybonus/JD_DailyBonus.js")
    mod_script(cookie,cookie2)
    luci.sys.call('uci set jd-dailybonus.@global[0].version="' .. version .. '"')
    luci.sys.call('uci commit jd-dailybonus')
    e.error = 0
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end