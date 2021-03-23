-- Copyright (C) 2020 jerrykuku <jerrykuku@gmail.com>
-- Licensed to the public under the GNU General Public License v3.
module('luci.controller.jd-dailybonus', package.seeall)
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
    entry({'admin', 'services', 'jd-dailybonus', 'realtime_log'}, call('get_log')) -- 获取实时日志
end

-- 执行程序
function run()
    local running = luci.sys.call("busybox ps -w | grep JD_DailyBonus.js | grep -v grep >/dev/null") == 0
    if not running then
        luci.sys.call('sh /usr/share/jd-dailybonus/newapp.sh -r')
    end
    luci.http.write('')
end

--检查更新
function check_update()
    local jd = 'jd-dailybonus'
    local e = {}
    local new_version = luci.sys.exec('sh /usr/share/jd-dailybonus/newapp.sh -n')
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
    local code = luci.sys.exec('sh /usr/share/jd-dailybonus/newapp.sh -u')
    e.error = code
    luci.http.prepare_content('application/json')
    luci.http.write_json(e)
end

local User_Agent='Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.111 Safari/537.36'
local Host='Host: plogin.m.jd.com'
local Accept='Accept: application/json, text/plain, */*'
local Accept_Language='Accept-Language: zh-cn'
local cookie='/tmp/jd_cookie'

function get_timestamp()
    return os.time()*1000
end

function get_s_token()
    luci.sys.exec('rm -f ' .. cookie)
    local timestamp = get_timestamp()
    local url =
        'https://plogin.m.jd.com/cgi-bin/mm/new_login_entrance?lang=chs&appid=300&returnurl=https://wq.jd.com/passport/LoginRedirect?state=' ..
        timestamp .. '&returnurl=https://home.m.jd.com/myJd/newhome.action?sceneval=2&ufc=&/myJd/home.action&source=wq_passport'
    local referer = 
        'https://plogin.m.jd.com/login/login?appid=300&returnurl=https://wq.jd.com/passport/LoginRedirect?state=' ..
        timestamp .. '&returnurl=https://home.m.jd.com/myJd/newhome.action?sceneval=2&ufc=&/myJd/home.action&source=wq_passport'
    local s_token = luci.sys.exec("echo -n $(wget-ssl --header='"..Accept.."' --header='"..Accept_Language.."' --header='"..Host.."' --referer='"..referer.."' --user-agent='"..User_Agent.."' --save-cookies="..cookie.." --keep-session-cookies -q -O - '"..url.."' | sed s/[[:space:]]//g | grep -oE '\"s_token\":\"(.+?)\"' | awk -F \\\" '{print $4}')")
    return s_token
end

--获取二维码
function qrcode()
    local timestamp = get_timestamp()
    local s_token = get_s_token()
    local url = 'https://plogin.m.jd.com/cgi-bin/m/tmauthreflogurl?s_token='..s_token..'&v='..timestamp..'&remember=true'
    local referer = 'https://plogin.m.jd.com/login/login?appid=300&returnurl=https://wq.jd.com/passport/LoginRedirect?state=' .. timestamp .. '&returnurl=https://home.m.jd.com/myJd/newhome.action?sceneval=2&ufc=&/myJd/home.action&source=wq_passport'
    local response = luci.sys.exec("echo -n $(wget-ssl --header='"..Accept.."' --header='"..Accept_Language.."' --header='"..Host.."' --referer='"..referer.."' --user-agent='"..User_Agent.."' --load-cookies="..cookie.." --save-cookies="..cookie.." --keep-session-cookies -q -O - '"..url.."')")
    local token = luci.sys.exec("echo -n $(echo \'"..response.."\' | grep -oE '\"token\":\"(.+?)\"' | awk -F \\\" '{print $4}')")
    local ou_state = luci.sys.exec("echo -n $(echo \'"..response.."\' | grep -oE '\"ou_state\":(\\d+)' | awk -F : '{print $2}')")
    local okl_token = luci.sys.exec("echo -n $(cat "..cookie.." | grep okl_token | awk '{print $7}')")
    local return_json = {
        qrcode_url = 'https://plogin.m.jd.com/cgi-bin/m/tmauth?appid=300&client_type=m&token=' .. token,
        check_url = 'https://plogin.m.jd.com/cgi-bin/m/tmauthchecktoken?&token=' .. token .. '&ou_state=' .. ou_state .. '&okl_token=' .. okl_token,
    }
    luci.http.prepare_content('application/json')
    luci.http.write_json(return_json)
end

--检测登录
function check_login()
    local uci = luci.model.uci.cursor()
    local data = luci.http.formvalue()
    local post_data = 'lang=chs&appid=300&source=wq_passport&returnurl=https://wqlogin2.jd.com/passport/LoginRedirect?state=1100399130787&returnurl=//home.m.jd.com/myJd/newhome.action?sceneval=2&ufc=&/myJd/home.action'
    local referer='https://plogin.m.jd.com/login/login?appid=300&returnurl=https://wqlogin2.jd.com/passport/LoginRedirect?state='
    local response = luci.sys.exec("echo -n $(wget-ssl --post-data='"..post_data.."' --header='"..Accept.."' --header='"..Accept_Language.."' --header='"..Host.."' --referer='"..referer.."' --user-agent='"..User_Agent.."' --load-cookies="..cookie.." --save-cookies="..cookie.." --keep-session-cookies -q -O - '"..data.check_url.."')")
    local return_json = {
        error = tonumber(luci.sys.exec("echo -n $(echo \'"..response.."\' | grep -oE '\"errcode\":(\\d+)' | awk -F : '{print $2}')")),
        msg = luci.sys.exec("echo -n $(echo \'"..response.."\' | grep -oE '\"message\":\"(.+?)\"' | awk -F \\\" '{print $4}')"),
    }
    if return_json.error == 0 then
        local pt_key = luci.sys.exec("echo -n $(cat "..cookie.." | grep pt_key | awk '{print $7}')")
        local pt_pin = luci.sys.exec("echo -n $(cat "..cookie.." | grep pt_pin | awk '{print $7}')")
        return_json.cookie = 'pt_key=' .. pt_key .. ';pt_pin=' .. pt_pin .. ';'
    end

    luci.http.prepare_content('application/json')
    luci.http.write_json(return_json)
end

function get_log()
    local fs = require "nixio.fs"
    local e = {}
    e.running = luci.sys.call("busybox ps -w | grep JD_DailyBonus.js | grep -v grep >/dev/null") == 0
    e.log = fs.readfile("/var/log/jd_dailybonus.log") or ""
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
