#!/usr/bin/lua

require 'luci.sys'
local luci = luci
local ucic = luci.model.uci.cursor()
local name = 'passwall'
local arg1 = arg[1]

local log = function(...)
    if arg1 then
        local result = os.date("%Y-%m-%d %H:%M:%S: ") ..
                           table.concat({...}, " ")
        if arg1 == "log" then
            local f, err = io.open("/var/log/passwall.log", "a")
            if f and err == nil then
                f:write(result .. "\n")
                f:close()
            end
        elseif arg1 == "print" then
            print(result)
        end
    end
end

-- trim
local function trim(text)
    if not text or text == "" then return "" end
    return (string.gsub(text, "^%s*(.-)%s*$", "%1"))
end
-- wget
local function wget(url, file)
	local cmd = "/usr/bin/wget --no-check-certificate -t 3 -T 10 -O"
	if file then
		cmd = cmd .. " " .. file .. " " .. url
	else
		cmd = cmd .. "- " .. url
	end
	local stdout = luci.sys.exec(cmd)
    return trim(stdout)
end

local rule_path = "/usr/share/" .. name .. "/rules"
local url_main = "https://raw.githubusercontent.com/hq450/fancyss/master/rules"
local reboot = 0
local gfwlist_update = 0
local chnroute_update = 0
local chnlist_update = 0
if arg[2] then
	if arg[2]:find("gfwlist") then
		gfwlist_update = 1
    end
	if arg[2]:find("chnroute") then
		chnroute_update = 1
    end
	if arg[2]:find("chnlist") then
		chnlist_update = 1
	end
else
	gfwlist_update = ucic:get_first(name, 'global_rules', "gfwlist_update", 1)
	chnroute_update = ucic:get_first(name, 'global_rules', "chnroute_update", 1)
	chnlist_update = ucic:get_first(name, 'global_rules', "chnlist_update", 1)
end
if gfwlist_update == 0 and chnroute_update == 0 and chnlist_update == 0 then
	os.exit(0)
end
local version = wget(url_main .. "/version1")
if version then
	if tonumber(gfwlist_update) == 1 then
		local gfwlist = luci.sys.exec("echo -n $(echo '" .. version .. "' | sed -n 1p)")
		local new_version = luci.sys.exec("echo -n $(echo '" .. gfwlist .. "' | awk '{print $1}')")
		local new_md5 = luci.sys.exec("echo -n $(echo '" .. gfwlist .. "' | awk '{print $3}')")
		if new_version ~= "" and new_md5 ~= "" then
			local old_version = ucic:get_first(name, 'global_rules', "gfwlist_version", "nil")
			local old_md5 = luci.sys.exec("echo -n $(md5sum " .. rule_path .. "/gfwlist.conf | awk '{print $1}')")
			if old_md5 ~= new_md5 or old_version ~= new_version then
				log("开始更新gfwlist...")
				wget(url_main .. "/gfwlist.conf", "/tmp/gfwlist_tmp")
				local download_md5 = luci.sys.exec("echo -n $([ -f '/tmp/gfwlist_tmp' ] && md5sum /tmp/gfwlist_tmp | awk '{print $1}')")
				if download_md5 == new_md5 then
					luci.sys.exec("mv -f /tmp/gfwlist_tmp " .. rule_path .. "/gfwlist.conf")
					ucic:set(name, ucic:get_first(name, 'global_rules'), "gfwlist_version", new_version)
					reboot = 1
					log("更新gfwlist成功...")
				end
			else
				log("gfwlist版本一致，不用更新。")
			end
		else
			log("gfwlist文件下载失败！")
		end
	end

	if tonumber(chnroute_update) == 1 then
		local chnroute = luci.sys.exec("echo -n $(echo '" .. version .. "' | sed -n 2p)")
		local new_version = luci.sys.exec("echo -n $(echo '" .. chnroute .. "' | awk '{print $1}')")
		local new_md5 = luci.sys.exec("echo -n $(echo '" .. chnroute .. "' | awk '{print $3}')")
		if new_version ~= "" and new_md5 ~= "" then
			local old_version = ucic:get_first(name, 'global_rules', "chnroute_version", "nil")
			local old_md5 = luci.sys.exec("echo -n $(md5sum " .. rule_path .. "/chnroute | awk '{print $1}')")
			if old_md5 ~= new_md5 or old_version ~= new_version then
				log("开始更新chnroute...")
				wget(url_main .. "/chnroute.txt", "/tmp/chnroute_tmp")
				local download_md5 = luci.sys.exec("echo -n $([ -f '/tmp/chnroute_tmp' ] && md5sum /tmp/chnroute_tmp | awk '{print $1}')")
				if download_md5 == new_md5 then
					luci.sys.exec("mv -f /tmp/chnroute_tmp " .. rule_path .. "/chnroute")
					ucic:set(name, ucic:get_first(name, 'global_rules'), "chnroute_version", new_version)
					reboot = 1
					log("更新chnroute成功...")
				end
			else
				log("chnroute版本一致，不用更新。")
			end
		else
			log("chnroute文件下载失败！")
		end
	end

	if tonumber(chnlist_update) == 1 then
		local chnlist = luci.sys.exec("echo -n $(echo '" .. version .. "' | sed -n 4p)")
		local new_version = luci.sys.exec("echo -n $(echo '" .. chnlist .. "' | awk '{print $1}')")
		local new_md5 = luci.sys.exec("echo -n $(echo '" .. chnlist .. "' | awk '{print $3}')")
		if new_version ~= "" and new_md5 ~= "" then
			local old_version = ucic:get_first(name, 'global_rules', "chnlist_version", "nil")
			local old_md5 = luci.sys.exec("echo -n $(md5sum " .. rule_path .. "/chnlist | awk '{print $1}')")
			if old_md5 ~= new_md5 or old_version ~= new_version then
				log("开始更新chnlist...")
				wget(url_main .. "/cdn.txt", "/tmp/chnlist_tmp")
				local download_md5 = luci.sys.exec("echo -n $([ -f '/tmp/chnlist_tmp' ] && md5sum /tmp/chnlist_tmp | awk '{print $1}')")
				if download_md5 == new_md5 then
					luci.sys.exec("mv -f /tmp/chnlist_tmp " .. rule_path .. "/chnlist")
					ucic:set(name, ucic:get_first(name, 'global_rules'), "chnlist_version", new_version)
					reboot = 1
					log("更新chnlist成功...")
				end
			else
				log("chnlist版本一致，不用更新。")
			end
		else
			log("chnlist文件下载失败！")
		end
	end
end
ucic:set(name, ucic:get_first(name, 'global_rules'), "gfwlist_update", gfwlist_update)
ucic:set(name, ucic:get_first(name, 'global_rules'), "chnroute_update", chnroute_update)
ucic:set(name, ucic:get_first(name, 'global_rules'), "chnlist_update", chnlist_update)
ucic:save(name)
luci.sys.call("uci commit " .. name)

if reboot == 1 then
	log("重启服务，应用新的规则。")
	luci.sys.call("/usr/share/" .. name .. "/iptables.sh flush_ipset &&  /etc/init.d/" .. name .. " restart")
end