#!/usr/bin/lua
------------------------------------------------
-- This file is part of the luci-app-ssr-plus update.lua
-- By Mattraks
------------------------------------------------

require 'nixio'
require 'luci.util'
require 'luci.jsonc'
require 'luci.sys'
local icount = 0
local ucic = luci.model.uci.cursor()

local log = function(...)
    print(os.date('%Y-%m-%d %H:%M:%S ') .. table.concat({...}, ' '))
end

log('正在更新【GFW列表】数据库')
if nixio.fs.access('/usr/bin/wget-ssl') then
    refresh_cmd = 'wget-ssl --no-check-certificate https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt -O /tmp/gfw.b64'
else
    refresh_cmd = 'wget -O /tmp/gfw.b64 http://iytc.net/tools/list.b64'
end
sret = luci.sys.call(refresh_cmd .. ' 2>/dev/null')
if sret == 0 then
    luci.sys.call('/usr/bin/vssr-gfw')
    icount = luci.sys.exec('cat /tmp/gfwnew.txt | wc -l')
    if tonumber(icount) > 1000 then
        oldcount = luci.sys.exec('cat /etc/vssr/gfw_list.conf | wc -l')
        if tonumber(icount) ~= tonumber(oldcount) then
            luci.sys.exec('cp -f /tmp/gfwnew.txt /etc/vssr/gfw_list.conf')
            --			retstring=tostring(math.ceil(tonumber(icount)/2))
            log('更新成功！ 新的总纪录数：' .. icount)
        else
            log('你已经是最新数据，无需更新！')
        end
    else
        log('更新失败！')
    end
    luci.sys.exec('rm -f /tmp/gfwnew.txt')
else
    log('更新失败！')
end

log('正在更新【国内IP段】数据库')
refresh_cmd ="wget -O- 'https://ispip.clang.cn/all_cn.txt' > /tmp/china_ssr.txt 2>/dev/null"
sret = luci.sys.call(refresh_cmd)
icount = luci.sys.exec('cat /tmp/china_ssr.txt | wc -l')
if sret == 0 then
    icount = luci.sys.exec('cat /tmp/china_ssr.txt | wc -l')
    if tonumber(icount) > 1000 then
        oldcount = luci.sys.exec('cat /etc/vssr/china_ssr.txt | wc -l')
        if tonumber(icount) ~= tonumber(oldcount) then
            luci.sys.exec('cp -f /tmp/china_ssr.txt /etc/vssr/china_ssr.txt')
            --			retstring=tostring(math.ceil(tonumber(icount)/2))
            log('更新成功！ 新的总纪录数：' .. icount)
        else
            log('你已经是最新数据，无需更新！')
        end
    else
        log('更新失败！')
    end
    luci.sys.exec('rm -f /tmp/china_ssr.txt')
else
    log('更新失败！')
end

-- --[[
if ucic:get_first('vssr', 'global', 'adblock', '') == '1' then
    log('正在更新【广告屏蔽】数据库')
    local need_process = 0
    if nixio.fs.access('/usr/bin/wget-ssl') then
        refresh_cmd = 'wget-ssl --no-check-certificate -O - https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt > /tmp/adnew.conf'
        need_process = 1
    else
        refresh_cmd = 'wget -O /tmp/ad.conf http://iytc.net/tools/ad.conf'
    end
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
                --		retstring=tostring(math.ceil(tonumber(icount)))
                if oldcount == 0 then
                    luci.sys.call('/etc/init.d/dnsmasq restart')
                end
                log('更新成功！ 新的总纪录数：' .. icount)
            else
                log('你已经是最新数据，无需更新！')
            end
        else
            log('更新失败！')
        end
        luci.sys.exec('rm -f /tmp/ad.conf')
    else
        log('更新失败！')
    end
end
-- --]]
