-- Copyright (C) 2020 jerrykuku <jerrykuku@gmail.com>
-- Licensed to the public under the GNU General Public License v3.
module("luci.controller.jd-dailybonus", package.seeall)
function index() 
    if not nixio.fs.access("/etc/config/jd-dailybonus") then 
        return 
    end

    entry({"admin", "services", "jd-dailybonus"}, alias("admin", "services", "jd-dailybonus", "client"), _("JD-DailyBonus"), 10).dependent = true -- 首页
    entry({"admin", "services", "jd-dailybonus", "client"}, cbi("jd-dailybonus/client"),_("Client"), 10).leaf = true -- 基本设置
    entry({"admin", "services", "jd-dailybonus", "log"},form("jd-dailybonus/log"),_("Log"), 30).leaf = true -- 日志页面
    entry({"admin", "services", "jd-dailybonus", "script"},form("jd-dailybonus/script"),_("Script"), 20).leaf = true -- 直接配置脚本
    entry({"admin", "services", "jd-dailybonus", "run"}, call("run")) -- 执行程序
    entry({"admin", "services", "jd-dailybonus", "update"}, call("update")) -- 执行更新
    entry({"admin", "services", "jd-dailybonus", "check_update"}, call("check_update")) -- 检查更新
end


-- 执行程序

function run()
    local e = {}
    local uci = luci.model.uci.cursor()
    local cookie = luci.http.formvalue("cookies")
    local cookie2 = luci.http.formvalue("cookies2")
    local auto_update = luci.http.formvalue("auto_update")
    local auto_update_time = luci.http.formvalue("auto_update_time")
    local auto_run = luci.http.formvalue("auto_run")
    local auto_run_time = luci.http.formvalue("auto_run_time")
    local stop = luci.http.formvalue("stop")
    local serverchan = luci.http.formvalue("serverchan")
    local failed = luci.http.formvalue("failed")
    local name = ""
    uci:foreach("vssr", "global", function(s) name = s[".name"] end)

    if cookie ~= " " then
        uci:set("jd-dailybonus", '@global[0]', 'auto_update', auto_update)
        uci:set("jd-dailybonus", '@global[0]', 'auto_update_time', auto_update_time)
        uci:set("jd-dailybonus", '@global[0]', 'auto_run', auto_run)
        uci:set("jd-dailybonus", '@global[0]', 'auto_run_time', auto_run_time)
        uci:set("jd-dailybonus", '@global[0]', 'stop', stop)
        uci:set("jd-dailybonus", '@global[0]', 'cookie', cookie)
        uci:set("jd-dailybonus", '@global[0]', 'cookie2', cookie2)
        uci:set("jd-dailybonus", '@global[0]', 'serverchan', serverchan)
        uci:set("jd-dailybonus", '@global[0]', 'failed', failed)
        uci:save("jd-dailybonus")
        uci:commit("jd-dailybonus")
        luci.sys.call("/usr/share/jd-dailybonus/newapp.sh -r")
        luci.sys.call("/usr/share/jd-dailybonus/newapp.sh -a")
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
    local new_version = luci.sys.exec("/usr/share/jd-dailybonus/newapp.sh -n")
    e.new_version = new_version
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
    --下载脚本
    local code = luci.sys.exec("/usr/share/jd-dailybonus/newapp.sh -u")
    e.error = code
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end