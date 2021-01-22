-- Copyright (C) 2021 jerrykuku <jerrykuku@gmail.com>
-- Licensed to the public under the GNU General Public License v3.

local uci = require 'luci.model.uci'.cursor()
local config = 'jd-dailybonus'
package.path = package.path .. ';/usr/share/jd-dailybonus/?.lua'
local requests = require 'requests'
local socket = require 'socket'
local tinsert = table.insert
local ssub, slen, schar, sbyte, sformat, sgsub = string.sub, string.len, string.char, string.byte, string.format, string.gsub
local User_Agent = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.111 Safari/537.36'
local Host = 'plogin.m.jd.com'
local jd_cookie = {}

--打印table
function print_r(t)
    local print_r_cache = {}
    local function sub_print_r(t, indent)
        if (print_r_cache[tostring(t)]) then
            print(indent .. '*' .. tostring(t))
        else
            print_r_cache[tostring(t)] = true
            if (type(t) == 'table') then
                for pos, val in pairs(t) do
                    if (type(val) == 'table') then
                        print(indent .. '[' .. pos .. '] => ' .. tostring(t) .. ' {')
                        sub_print_r(val, indent .. string.rep(' ', string.len(pos) + 8))
                        print(indent .. string.rep(' ', string.len(pos) + 6) .. '}')
                    elseif (type(val) == 'string') then
                        print(indent .. '[' .. pos .. '] => "' .. val .. '"')
                    else
                        print(indent .. '[' .. pos .. '] => ' .. tostring(val))
                    end
                end
            else
                print(indent .. tostring(t))
            end
        end
    end
    if (type(t) == 'table') then
        print(tostring(t) .. ' {')
        sub_print_r(t, '  ')
        print('}')
    else
        sub_print_r(t, '  ')
    end
    print()
end

-- 分割字符串
local function split(full, sep)
    full = full:gsub('%z', '') -- 这里不是很清楚 有时候结尾带个\0
    local off, result = 1, {}
    while true do
        local nStart, nEnd = full:find(sep, off)
        if not nEnd then
            local res = ssub(full, off, slen(full))
            if #res > 0 then -- 过滤掉 \0
                tinsert(result, res)
            end
            break
        else
            tinsert(result, ssub(full, off, nStart - 1))
            off = nEnd + 1
        end
    end
    return result
end

--获取unix时间戳
function jd_cookie.timestamp()
    return math.floor(socket.gettime() * 1000)
end

--延时函数
function sleep(n)
    os.execute('sleep ' .. n)
end

--解析cookie
function praseSetCookies(rcookie)
    local tbl = {}
    for k, v in rcookie:gmatch('([^;%s]+)=([^;]+)') do
        tbl[k] = v
    end
    return tbl
end

--获取二维码链接和检测URL
function jd_cookie.get_qrcode()
    local timeStamp = jd_cookie.timestamp()
    local url =
        'https://plogin.m.jd.com/cgi-bin/mm/new_login_entrance?lang=chs&appid=300&returnurl=https://wq.jd.com/passport/LoginRedirect?state=' ..
        timeStamp .. '&returnurl=https://home.m.jd.com/myJd/newhome.action?sceneval=2&ufc=&/myJd/home.action&source=wq_passport'
    local xformHeaders = {
        ['Content-Type'] = 'application/x-www-form-urlencoded',
        ['Connection'] = 'Keep-Alive',
        ['Accept'] = 'application/json, text/plain, */*',
        ['Accept-Language'] = 'zh-cn',
        ['Referer'] = 'https://plogin.m.jd.com/login/login?appid=300&returnurl=https://wq.jd.com/passport/LoginRedirect?state=' ..
            timeStamp .. '&returnurl=https://home.m.jd.com/myJd/newhome.action?sceneval=2&ufc=&/myJd/home.action&source=wq_passport',
        ['User-Agent'] = User_Agent,
        ['Host'] = Host
    }
    local response = requests.get {url, headers = xformHeaders}
    local s_token = response.json().s_token
    local dat = split(string.gsub(response.headers['set-cookie'], ' DOMAIN=.plogin.m.jd.com', ''), ';')
    local cookies = string.gsub(dat[1] .. dat[4] .. dat[6] .. dat[8], ',', ';') .. ';'

    local timeStamp = jd_cookie.timestamp()
    local url = 'https://plogin.m.jd.com/cgi-bin/m/tmauthreflogurl?s_token=' .. s_token .. '&v=' .. timeStamp .. '&remember=true'
    local xformHeaders = {
        ['Content-Type'] = 'application/x-www-form-urlencoded',
        ['Connection'] = 'Keep-Alive',
        ['Accept'] = 'application/json, text/plain, */*',
        ['Accept-Language'] = 'zh-cn',
        ['Referer'] = 'https://plogin.m.jd.com/login/login?appid=300&returnurl=https://wq.jd.com/passport/LoginRedirect?state=' ..
            timeStamp .. '&returnurl=https://home.m.jd.com/myJd/newhome.action?sceneval=2&ufc=&/myJd/home.action&source=wq_passport',
        ['User-Agent'] = User_Agent,
        ['Host'] = Host
    }
    local response = requests.post {url, cookies = cookies, headers = xformHeaders}
    local json_body, error = response.json()
    local token = json_body['token']
    local ou_state = json_body['ou_state']
    local okl_token = praseSetCookies(response.headers['set-cookie']).okl_token
    local return_json = {
        qrcode_url = 'https://plogin.m.jd.com/cgi-bin/m/tmauth?appid=300&client_type=m&token=' .. token,
        check_url = 'https://plogin.m.jd.com/cgi-bin/m/tmauthchecktoken?&token=' .. token .. '&ou_state=' .. ou_state .. '&okl_token=' .. okl_token,
        cookies = cookies
    }
    return return_json
end

--检测扫码状态 并返回cookie
function jd_cookie.checkLogin(url, cookies)
    local xformHeaders = {
        ['Content-Type'] = 'application/x-www-form-urlencoded',
        ['Connection'] = 'Keep-Alive',
        ['Accept'] = 'application/json, text/plain, */*',
        ['Accept-Language'] = 'zh-cn',
        ['Referer'] = 'https://plogin.m.jd.com/login/login?appid=300&returnurl=https://wqlogin2.jd.com/passport/LoginRedirect?state=',
        ['User-Agent'] = User_Agent,
        ['Host'] = Host
    }
    local data =
        'lang=chs&appid=300&source=wq_passport&returnurl=https://wqlogin2.jd.com/passport/LoginRedirect?state=1100399130787&returnurl=//home.m.jd.com/myJd/newhome.action?sceneval=2&ufc=&/myJd/home.action'
    local response = requests.post {url, data = data, cookies = cookies, headers = xformHeaders}
    --local cookie = jd_cookie.praseSetCookies(response.headers['set-cookie'])
    local json = response.json()
    local return_json = {}
    if json.errcode == 0 then
        local ucookie = praseSetCookies(response.headers['set-cookie'])
        return_json = {
            error = 0,
            cookie = ucookie
        }
    else
        return_json = {
            error = json.errcode,
            msg = json.message
        }
    end

    return return_json
end

return jd_cookie
