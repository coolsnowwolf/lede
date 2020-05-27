#!/usr/bin/lua
------------------------------------------------
-- This file is part of the luci-app-ssr-plus update.lua
-- By Mattraks
------------------------------------------------
require "luci.sys"
require "luci.model.uci"
local icount = 0
local uci = luci.model.uci.cursor()
local log = function(...)
	print(os.date("%Y-%m-%d %H:%M:%S ") .. table.concat({ ... }, " "))
end

local function update(url, file, type, file2)
	local Num = 1
	refresh_cmd = "wget-ssl --no-check-certificate -t 3 -T 10 -O- " .. url .. " > /tmp/ssr-update." .. type
	sret = luci.sys.call(refresh_cmd .. " 2>/dev/null")
	if sret == 0 then
		if type == "gfw_data" then
			luci.sys.call("/usr/bin/ssr-gfw " .. type)
			Num = 2
		end
		if type == "ad_data" then
			luci.sys.call("/usr/bin/ssr-ad " .. type)
		end
		local new_md5 = luci.sys.exec("echo -n $([ -f '/tmp/ssr-update." .. type .. "' ] && md5sum /tmp/ssr-update." .. type .. " | awk '{print $1}')")
		local old_md5 = luci.sys.exec("echo -n $([ -f '" .. file .. "' ] && md5sum " .. file .. " | awk '{print $1}')")
		if new_md5 == old_md5 then
			log("你已经是最新数据，无需更新！")
		else
			icount = luci.sys.exec("cat /tmp/ssr-update." .. type .. " | wc -l")
			luci.sys.exec("cp -f /tmp/ssr-update." .. type .. " " .. file)
			if file2 then luci.sys.exec("cp -f /tmp/ssr-update." .. type .. " " .. file2) end
			log("更新成功！ 新的总纪录数：" .. tostring(tonumber(icount)/Num))
		end
	else
		log("更新失败！")
	end
	luci.sys.exec("rm -f /tmp/ssr-update." .. type)
end

log("正在更新【GFW列表】数据库")
update(uci:get_first("shadowsocksr", "global", "gfwlist_url", "https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt"), "/etc/ssr/gfw_list.conf", "gfw_data", "/tmp/dnsmasq.ssr/gfw_list.conf")
log("正在更新【国内IP段】数据库")
update(uci:get_first("shadowsocksr", "global", "chnroute_url","https://ispip.clang.cn/all_cn.txt"), "/etc/ssr/china_ssr.txt", "cnip")
if uci:get_first("shadowsocksr", "global", "adblock","0") == "1" then
	log("正在更新【广告屏蔽】数据库")
	update(uci:get_first("shadowsocksr", "global", "adblock_url","https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt"), "/etc/ssr/ad.conf", "ad_data", "/tmp/dnsmasq.ssr/ad.conf")
end
-- log("正在更新【Netflix IP段】数据库")
-- update(uci:get_first("shadowsocksr", "global", "nfip_url","https://raw.githubusercontent.com/QiuSimons/Netflix_IP/master/NF_only.txt"), "/etc/ssr/netflixip.list", "nfip_data")
