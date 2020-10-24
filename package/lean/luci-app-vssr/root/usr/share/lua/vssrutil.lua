#!/usr/bin/lua

------------------------------------------------
-- This file is converter ip to country iso code
-- @author Jerryk <jerrykuku@qq.com>
------------------------------------------------
local _M = {}

-- Get country iso code with remark or host
-- Return String:iso_code
function _M.get_flag(remark, host)
    local nixio = require "nixio"
    local iso_table = {
        "US", "US", "US", "US", "US", "HK", "HK", "TW", "TW", "TW", "TW", "TW",
        "JP", "JP", "JP", "GB", "GB", "DE", "DE", "FR", "IN", "TR", "SG", "SG",
        "KR", "KR", "KR", "KR", "RU", "IE", "CA", "CA", "AU", "CH", "AU", "AR","RU",
        "AC", "AD", "AE", "AF", "AG", "AI", "AL", "AM", "AO", "AQ", "AR", "AS",
        "AT", "AU", "AW", "AX", "AZ", "BA", "BB", "BD", "BE", "BF", "BG", "BH",
        "BI", "BJ", "BL", "BM", "BN", "BO", "BQ", "BR", "BS", "BT", "BV", "BW",
        "BY", "BZ", "CA", "CC", "CD", "CF", "CG", "CH", "CI", "CK", "CL", "CM",
        "CN", "CO", "CP", "CR", "CU", "CV", "CW", "CX", "CY", "CZ", "DE", "DG",
        "DJ", "DK", "DM", "DO", "DZ", "EA", "EC", "EE", "EG", "EH", "ER", "ES",
        "ET", "EU", "FI", "FJ", "FK", "FM", "FO", "FR", "GA", "GB", "GD", "GE",
        "GF", "GG", "GH", "GI", "GL", "GM", "GN", "GP", "GQ", "GR", "GS", "GT",
        "GU", "GW", "GY", "HK", "HM", "HN", "HR", "HT", "HU", "IC", "ID", "IE",
        "IL", "IM", "IN", "IO", "IQ", "IR", "IS", "IT", "JE", "JM", "JO", "JP",
        "KE", "KG", "KH", "KI", "KM", "KN", "KP", "KR", "KW", "KY", "KZ", "LA",
        "LB", "LC", "LI", "LK", "LR", "LS", "LT", "LU", "LV", "LY", "MA", "MC",
        "MD", "ME", "MF", "MG", "MH", "MK", "ML", "MM", "MN", "MO", "MP", "MQ",
        "MR", "MS", "MT", "MU", "MV", "MW", "MX", "MY", "MZ", "NA", "NC", "NE",
        "NF", "NG", "NI", "NL", "NO", "NP", "NR", "NU", "NZ", "OM", "PA", "PE",
        "PF", "PG", "PH", "PK", "PL", "PM", "PN", "PR", "PS", "PT", "PW", "PY",
        "QA", "RE", "RO", "RS", "RU", "RW", "SA", "SB", "SC", "SD", "SE", "SG",
        "SH", "SI", "SJ", "SK", "SL", "SM", "SN", "SO", "SR", "SS", "ST", "SV",
        "SX", "SY", "SZ", "TA", "TC", "TD", "TF", "TG", "TH", "TJ", "TK", "TL",
        "TM", "TN", "TO", "TR", "TT", "TV", "TW", "TZ", "UA", "UG", "UM", "UN",
        "US", "UY", "UZ", "VA", "VC", "VE", "VG", "VI", "VN", "VU", "WF", "WS",
        "XK", "YE", "YT", "ZA", "ZM", "ZW"
    }

    local emoji_table = {
        "美国", "US", "洛杉矶","芝加哥","达拉斯", "香港","HK", "台湾", "TW", "TAIWAN","彰化","台北",
        "日本", "JP", "JAPAN", "英国", "UK", "德国", "DE", "法国", "印度", "土耳其", "新加坡", "SG",
        "韩国", "首尔", "KR", "KOREA", "俄罗斯", "爱尔兰", "CANADA", "加拿大","SYDNEY", "瑞士", "澳大利亚", "阿根廷","RU",
        "🇦🇨", "🇦🇩", "🇦🇪", "🇦🇫", "🇦🇬", "🇦🇮",
        "🇦🇱", "🇦🇲", "🇦🇴", "🇦🇶", "🇦🇷", "🇦🇸",
        "🇦🇹", "🇦🇺", "🇦🇼", "🇦🇽", "🇦🇿", "🇧🇦",
        "🇧🇧", "🇧🇩", "🇧🇪", "🇧🇫", "🇧🇬", "🇧🇭",
        "🇧🇮", "🇧🇯", "🇧🇱", "🇧🇲", "🇧🇳", "🇧🇴",
        "🇧🇶", "🇧🇷", "🇧🇸", "🇧🇹", "🇧🇻", "🇧🇼",
        "🇧🇾", "🇧🇿", "🇨🇦", "🇨🇨", "🇨🇩", "🇨🇫",
        "🇨🇬", "🇨🇭", "🇨🇮", "🇨🇰", "🇨🇱", "🇨🇲",
        "🇨🇳", "🇨🇴", "🇨🇵", "🇨🇷", "🇨🇺", "🇨🇻",
        "🇨🇼", "🇨🇽", "🇨🇾", "🇨🇿", "🇩🇪", "🇩🇬",
        "🇩🇯", "🇩🇰", "🇩🇲", "🇩🇴", "🇩🇿", "🇪🇦",
        "🇪🇨", "🇪🇪", "🇪🇬", "🇪🇭", "🇪🇷", "🇪🇸",
        "🇪🇹", "🇪🇺", "🇫🇮", "🇫🇯", "🇫🇰", "🇫🇲",
        "🇫🇴", "🇫🇷", "🇬🇦", "🇬🇧", "🇬🇩", "🇬🇪",
        "🇬🇫", "🇬🇬", "🇬🇭", "🇬🇮", "🇬🇱", "🇬🇲",
        "🇬🇳", "🇬🇵", "🇬🇶", "🇬🇷", "🇬🇸", "🇬🇹",
        "🇬🇺", "🇬🇼", "🇬🇾", "🇭🇰", "🇭🇲", "🇭🇳",
        "🇭🇷", "🇭🇹", "🇭🇺", "🇮🇨", "🇮🇩", "🇮🇪",
        "🇮🇱", "🇮🇲", "🇮🇳", "🇮🇴", "🇮🇶", "🇮🇷",
        "🇮🇸", "🇮🇹", "🇯🇪", "🇯🇲", "🇯🇴", "🇯🇵",
        "🇰🇪", "🇰🇬", "🇰🇭", "🇰🇮", "🇰🇲", "🇰🇳",
        "🇰🇵", "🇰🇷", "🇰🇼", "🇰🇾", "🇰🇿", "🇱🇦",
        "🇱🇧", "🇱🇨", "🇱🇮", "🇱🇰", "🇱🇷", "🇱🇸",
        "🇱🇹", "🇱🇺", "🇱🇻", "🇱🇾", "🇲🇦", "🇲🇨",
        "🇲🇩", "🇲🇪", "🇲🇫", "🇲🇬", "🇲🇭", "🇲🇰",
        "🇲🇱", "🇲🇲", "🇲🇳", "🇲🇴", "🇲🇵", "🇲🇶",
        "🇲🇷", "🇲🇸", "🇲🇹", "🇲🇺", "🇲🇻", "🇲🇼",
        "🇲🇽", "🇲🇾", "🇲🇿", "🇳🇦", "🇳🇨", "🇳🇪",
        "🇳🇫", "🇳🇬", "🇳🇮", "🇳🇱", "🇳🇴", "🇳🇵",
        "🇳🇷", "🇳🇺", "🇳🇿", "🇴🇲", "🇵🇦", "🇵🇪",
        "🇵🇫", "🇵🇬", "🇵🇭", "🇵🇰", "🇵🇱", "🇵🇲",
        "🇵🇳", "🇵🇷", "🇵🇸", "🇵🇹", "🇵🇼", "🇵🇾",
        "🇶🇦", "🇷🇪", "🇷🇴", "🇷🇸", "🇷🇺", "🇷🇼",
        "🇸🇦", "🇸🇧", "🇸🇨", "🇸🇩", "🇸🇪", "🇸🇬",
        "🇸🇭", "🇸🇮", "🇸🇯", "🇸🇰", "🇸🇱", "🇸🇲",
        "🇸🇳", "🇸🇴", "🇸🇷", "🇸🇸", "🇸🇹", "🇸🇻",
        "🇸🇽", "🇸🇾", "🇸🇿", "🇹🇦", "🇹🇨", "🇹🇩",
        "🇹🇫", "🇹🇬", "🇹🇭", "🇹🇯", "🇹🇰", "🇹🇱",
        "🇹🇲", "🇹🇳", "🇹🇴", "🇹🇷", "🇹🇹", "🇹🇻",
        "🇹🇼", "🇹🇿", "🇺🇦", "🇺🇬", "🇺🇲", "🇺🇳",
        "🇺🇸", "🇺🇾", "🇺🇿", "🇻🇦", "🇻🇨", "🇻🇪",
        "🇻🇬", "🇻🇮", "🇻🇳", "🇻🇺", "🇼🇫", "🇼🇸",
        "🇽🇰", "🇾🇪", "🇾🇹", "🇿🇦", "🇿🇲", "🇿🇼"
    }

    local iso_code = nil
    if (remark ~= nil) then
        for i, v in pairs(emoji_table) do
            if (string.find(string.lower(remark), string.lower(v))) then
                iso_code = string.lower(iso_table[i])
                break
            end
        end
    end

    if (iso_code == nil ) then
        if( host ~= "") then
            local ret = nixio.getaddrinfo(_M.trim(host), "any")
            if(ret == nil) then
                iso_code = "un"
            else
                local hostip = ret[1].address
                local status,code = pcall(_M.get_iso,hostip)
                if(status) then
                    iso_code = code
                else
                    iso_code = "un"
                end
            end
        else
            iso_code = "un"
        end
    end
    return string.gsub(iso_code, '\n', '')
end

function _M.get_iso(ip)
    local mm = require 'maxminddb'
    local db = mm.open('/usr/share/vssr/GeoLite2-Country.mmdb')
    local res = db:lookup(ip)
    return string.lower(res:get("country", "iso_code"))
end

function _M.get_cname(ip)
    local mm = require 'maxminddb'
    local db = mm.open('/usr/share/vssr/GeoLite2-Country.mmdb')
    local res = db:lookup(ip)
    return string.lower(res:get("country", "names", "zh-CN"))
end


-- Get status of conncet to any site with host and port
-- Return String:true or nil
function _M.check_site(host, port)
    local nixio = require "nixio"
    local socket = nixio.socket("inet", "stream")
    socket:setopt("socket", "rcvtimeo", 2)
    socket:setopt("socket", "sndtimeo", 2)
    local ret = socket:connect(host, port)
    socket:close()
    return ret
end

function _M.trim(text)
    if not text or text == "" then return "" end
    return (string.gsub(text, "^%s*(.-)%s*$", "%1"))
end

function _M.wget(url)
    local sys = require "luci.sys"
    local stdout = sys.exec(
                       'wget-ssl -q --user-agent="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.157 Safari/537.36" --no-check-certificate -t 3 -T 10 -O- "' ..
                           url .. '"')
    return _M.trim(stdout)
end

return _M

