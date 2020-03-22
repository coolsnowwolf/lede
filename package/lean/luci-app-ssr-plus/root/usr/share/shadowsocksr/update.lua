#!/usr/bin/lua
------------------------------------------------
-- This file is part of the luci-app-ssr-plus update.lua
-- By Mattraks
------------------------------------------------
require 'nixio'
require 'luci.util'
require 'luci.jsonc'
require 'luci.sys'
local icount =0
local uci = luci.model.uci.cursor()

local log = function(...)
	print(os.date("%Y-%m-%d %H:%M:%S ") .. table.concat({ ... }, " "))
end

log('正在更新【GFW列表】数据库')
refresh_cmd = "wget-ssl --no-check-certificate -O - https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt > /tmp/gfw.b64"
sret = luci.sys.call(refresh_cmd .. " 2>/dev/null")
if sret == 0 then
	luci.sys.call("/usr/bin/ssr-gfw")
	icount = luci.sys.exec("cat /tmp/gfwnew.txt | wc -l")
	if tonumber(icount) > 1000 then
		oldcount = luci.sys.exec("cat /etc/dnsmasq.ssr/gfw_list.conf | wc -l")
		if tonumber(icount) ~= tonumber(oldcount) then
			luci.sys.exec("cp -f /tmp/gfwnew.txt /etc/dnsmasq.ssr/gfw_list.conf")
			luci.sys.exec("cp -f /tmp/gfwnew.txt /tmp/dnsmasq.ssr/gfw_list.conf")
			log('更新成功！ 新的总纪录数：'.. icount)
		else
			log('你已经是最新数据，无需更新！')
		end
	else
		log('更新失败！')
	end
	luci.sys.exec("rm -f /tmp/gfwnew.txt")
else
	log('更新失败！')
end

log('正在更新【国内IP段】数据库')
refresh_cmd = "wget-ssl --no-check-certificate -O - ".. uci:get_first('shadowsocksr', 'global', 'chnroute_url','https://ispip.clang.cn/all_cn.txt') .." > /tmp/china_ssr.txt"
sret = luci.sys.call(refresh_cmd .. " 2>/dev/null")
icount = luci.sys.exec("cat /tmp/china_ssr.txt | wc -l")
if sret == 0 then
	icount = luci.sys.exec("cat /tmp/china_ssr.txt | wc -l")
	if tonumber(icount) > 1000 then
		oldcount = luci.sys.exec("cat /etc/china_ssr.txt | wc -l")
		if tonumber(icount) ~= tonumber(oldcount) then
			luci.sys.exec("cp -f /tmp/china_ssr.txt /etc/china_ssr.txt")
			log('更新成功！ 新的总纪录数：'.. icount)
		else
			log('你已经是最新数据，无需更新！')
		end
	else
		log('更新失败！')
	end
	luci.sys.exec("rm -f /tmp/china_ssr.txt")
else
	log('更新失败！')
end

if uci:get_first('shadowsocksr', 'global', 'adblock','0') == "1" then
	log('正在更新【广告屏蔽】数据库')
	refresh_cmd = "wget-ssl --no-check-certificate -O - ".. uci:get_first('shadowsocksr', 'global', 'adblock_url','https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt') .." > /tmp/adnew.conf"
	sret = luci.sys.call(refresh_cmd .. " 2>/dev/null")
	if sret == 0 then
		luci.sys.call("/usr/bin/ssr-ad")
		icount = luci.sys.exec("cat /tmp/ad.conf | wc -l")
		if tonumber(icount) > 100 then
			if nixio.fs.access("/etc/dnsmasq.ssr/ad.conf") then
				oldcount = luci.sys.exec("cat /etc/dnsmasq.ssr/ad.conf | wc -l")
			else
				oldcount = "0"
			end
			if tonumber(icount) ~= tonumber(oldcount) then
				luci.sys.exec("cp -f /tmp/ad.conf /etc/dnsmasq.ssr/ad.conf")
				luci.sys.exec("cp -f /tmp/ad.conf /tmp/dnsmasq.ssr/ad.conf")
				log('更新成功！ 新的总纪录数：'.. icount)
			else
				log('你已经是最新数据，无需更新！')
			end
		else
			log('更新失败！')
		end
		luci.sys.exec("rm -f /tmp/ad.conf")
	else
		log('更新失败！')
	end
end

--[[
log('正在更新【Netflix IP段】数据库')
refresh_cmd = "wget-ssl --no-check-certificate -O - ".. uci:get_first('shadowsocksr', 'global', 'nfip_url','https://raw.githubusercontent.com/QiuSimons/Netflix_IP/master/NF_only.txt') .." > /tmp/netflixip.list"
sret = luci.sys.call(refresh_cmd .. " 2>/dev/null")
if sret == 0 then
	luci.sys.call("/usr/bin/ssr-gfw")
	icount = luci.sys.exec("cat /tmp/netflixip.list | wc -l")
	if tonumber(icount) > 5 then
		oldcount = luci.sys.exec("cat /etc/config/netflixip.list | wc -l")
		if tonumber(icount) ~= tonumber(oldcount) then
			luci.sys.exec("cp -f /tmp/netflixip.list /etc/config/netflixip.list")
			log('更新成功！ 新的总纪录数：'.. icount)
		else
			log('你已经是最新数据，无需更新！')
		end
	else
		log('更新失败！')
	end
	luci.sys.exec("rm -f /tmp/netflixip.list")
else
	log('更新失败！')
end
--]]

luci.sys.call("/etc/init.d/dnsmasq reload")
