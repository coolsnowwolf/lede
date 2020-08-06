#!/usr/bin/lua

require 'luci.sys'
local luci = luci
local ucic = luci.model.uci.cursor()
local name = 'passwall'
local arg1 = arg[1]

local rule_path = "/usr/share/" .. name .. "/rules"
local url_main = 'https://cdn.jsdelivr.net/gh/hq450/fancyss/rules'
local reboot = 0
local gfwlist_update = 0
local chnroute_update = 0
local chnlist_update = 0

-- match comments/title/whitelist/ip address/excluded_domain
local comment_pattern = "^[!\\[@]+"
local ip_pattern = "^%d+%.%d+%.%d+%.%d+"
local domain_pattern = "([%w%-%_]+%.[%w%.%-%_]+)[%/%*]*"
local excluded_domain = {"apple.com", "sina.cn","sina.com.cn","baidu.com","byr.cn","jlike.com","weibo.com","zhongsou.com","youdao.com","sogou.com","so.com","soso.com","aliyun.com","taobao.com","jd.com","qq.com"}

-- gfwlist parameter
local mydnsip = '127.0.0.1'
local mydnsport = '7913'
local ipsetname = 'gfwlist'

-- custom url
local enable_custom_url = ucic:get_first(name, 'global_rules', "enable_custom_url", 0)
enable_custom_url = 1
local gfwlist_url = ucic:get_first(name, 'global_rules', "gfwlist_url", "https://cdn.jsdelivr.net/gh/Loukky/gfwlist-by-loukky/gfwlist.txt")
local chnroute_url = ucic:get_first(name, 'global_rules', "chnroute_url", "https://ispip.clang.cn/all_cn.txt")
local chnlist_url_1 = 'https://cdn.jsdelivr.net/gh/felixonmars/dnsmasq-china-list/accelerated-domains.china.conf'
local chnlist_url_2 = 'https://cdn.jsdelivr.net/gh/felixonmars/dnsmasq-china-list/apple.china.conf'
local chnlist_url_3 = 'https://cdn.jsdelivr.net/gh/felixonmars/dnsmasq-china-list/google.china.conf'

local bc='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'

-- base64decoding
local function base64_dec(data)
    data = string.gsub(data, '[^'..bc..'=]', '')
    return (data:gsub('.', function(x)
        if (x == '=') then return '' end
        local r,f='',(bc:find(x)-1)
        for i=6,1,-1 do r=r..(f%2^i-f%2^(i-1)>0 and '1' or '0') end
        return r;
    end):gsub('%d%d%d?%d?%d?%d?%d?%d?', function(x)
        if (#x ~= 8) then return '' end
        local c=0
        for i=1,8 do c=c+(x:sub(i,i)=='1' and 2^(8-i) or 0) end
        return string.char(c)
    end))
end

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

-- curl
local function curl(url, file)
	local cmd = "curl -skL -w %{http_code} --retry 3 --connect-timeout 3 '" .. url .. "'"
	if file then
		cmd = cmd .. " -o " .. file
	end
	local stdout = luci.sys.exec(cmd)

	if file then
		return tonumber(trim(stdout))
	else
		return trim(stdout)
	end
end

--获取gfwlist
local function fetch_gfwlist()
	--请求gfwlist
	local sret = curl(gfwlist_url, "/tmp/gfwlist.txt")
	if sret == 200 then
		--解码gfwlist
		local gfwlist = io.open("/tmp/gfwlist.txt", "r")
		local decode = base64_dec(gfwlist:read("*all"))
		gfwlist:close()
		--写回gfwlist
		gfwlist = io.open("/tmp/gfwlist.txt", "w")
		gfwlist:write(decode)
		gfwlist:close()
	end

	return sret;
end

--获取chnroute
local function fetch_chnroute()
	--请求chnroute
	local sret = curl(chnroute_url, "/tmp/chnroute_tmp")
	return sret;
end

--获取chnlist
local function fetch_chnlist()
	--请求chnlist
	local sret = 0
	local sret1 = curl(chnlist_url_1, "/tmp/chnlist_1")
	local sret2 = curl(chnlist_url_2, "/tmp/chnlist_2")
	local sret3 = curl(chnlist_url_3, "/tmp/chnlist_3")

	if sret1 == 200 and sret2 ==200 and sret3 == 200 then
		sret=200
	end
	return sret;
end

--check excluded domain
local function check_excluded_domain(value)
	for k,v in ipairs(excluded_domain) do
		if value:find(v) then
			return true
		end
	end
end

--gfwlist转码至dnsmasq格式
local function generate_gfwlist()
	local domains = {}
	local out = io.open("/tmp/gfwlist_tmp", "w")

	for line in io.lines("/tmp/gfwlist.txt") do
		if not (string.find(line, comment_pattern) or string.find(line, ip_pattern) or check_excluded_domain(line)) then
			local start, finish, match = string.find(line, domain_pattern)
			if (start) then
				domains[match] = true
			end
		end
	end

	for k,v in pairs(domains) do
		out:write(string.format("server=/.%s/%s#%s\n", k,mydnsip,mydnsport))
		out:write(string.format("ipset=/.%s/%s\n", k,ipsetname))
	end

	out:close()
end

--处理合并chnlist列表
local function generate_chnlist()
	local domains = {}
	local out = io.open("/tmp/cdn_tmp", "w")

	for line in io.lines("/tmp/chnlist_1") do
		local start, finish, match = string.find(line, domain_pattern)
		if (start) then
			domains[match] = true
		end
	end

	for line in io.lines("/tmp/chnlist_2") do
		local start, finish, match = string.find(line, domain_pattern)
		if (start) then
			domains[match] = true
		end
	end

	for line in io.lines("/tmp/chnlist_3") do
		local start, finish, match = string.find(line, domain_pattern)
		if (start) then
			domains[match] = true
		end
	end

	--写入临时文件
	for k,v in pairs(domains) do
		out:write(string.format("%s\n", k))
	end

	out:close()

	--删除重复条目并排序
	luci.sys.call("cat /tmp/cdn_tmp | sort -u > /tmp/chnlist_tmp")
end

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

log("开始更新规则...")
if tonumber(enable_custom_url) == 1 then
	local new_version = os.date("%Y-%m-%d")
	log("已启用自定义规则地址...")
	if tonumber(gfwlist_update) == 1 then
		log("开始更新gfwlist...")
		local old_md5 = luci.sys.exec("echo -n $(md5sum " .. rule_path .. "/gfwlist.conf | awk '{print $1}')")
		local status = fetch_gfwlist()
		if status == 200 then
			generate_gfwlist()
			local new_md5 = luci.sys.exec("echo -n $([ -f '/tmp/gfwlist_tmp' ] && md5sum /tmp/gfwlist_tmp | awk '{print $1}')")
			if old_md5 ~= new_md5 then
				luci.sys.exec("mv -f /tmp/gfwlist_tmp " .. rule_path .. "/gfwlist.conf")
				ucic:set(name, ucic:get_first(name, 'global_rules'), "gfwlist_version", new_version)
				reboot = 1
				log("更新gfwlist成功...")
			else
				log("gfwlist版本一致，不用更新。")
			end
		else
			log("gfwlist文件下载失败！")
		end
		os.remove("/tmp/gfwlist.txt")
		os.remove("/tmp/gfwlist_tmp")
	end

	if tonumber(chnroute_update) == 1 then
		log("开始更新chnroute...")
		local old_md5 = luci.sys.exec("echo -n $(md5sum " .. rule_path .. "/chnroute | awk '{print $1}')")
		local status = fetch_chnroute()
		if status == 200 then
			local new_md5 = luci.sys.exec("echo -n $([ -f '/tmp/chnroute_tmp' ] && md5sum /tmp/chnroute_tmp | awk '{print $1}')")
			if old_md5 ~= new_md5 then
				luci.sys.exec("mv -f /tmp/chnroute_tmp " .. rule_path .. "/chnroute")
				ucic:set(name, ucic:get_first(name, 'global_rules'), "chnroute_version", new_version)
				reboot = 1
				log("更新chnroute成功...")
			else
				log("chnroute版本一致，不用更新。")
			end
		else
			log("chnroute文件下载失败！")
		end
		os.remove("/tmp/chnroute_tmp")
	end
	
	if tonumber(chnlist_update) == 1 then
		log("开始更新chnlist...")
		local old_md5 = luci.sys.exec("echo -n $(md5sum " .. rule_path .. "/chnlist | awk '{print $1}')")
		local status = fetch_chnlist()
		if status == 200 then
			generate_chnlist()
			local new_md5 = luci.sys.exec("echo -n $([ -f '/tmp/chnlist_tmp' ] && md5sum /tmp/chnlist_tmp | awk '{print $1}')")
			if old_md5 ~= new_md5 then
				luci.sys.exec("mv -f /tmp/chnlist_tmp " .. rule_path .. "/chnlist")
				ucic:set(name, ucic:get_first(name, 'global_rules'), "chnlist_version", new_version)
				reboot = 1
				log("更新chnlist成功...")
			else
				log("chnlist版本一致，不用更新。")
			end
		else
			log("chnlist文件下载失败！")
		end
		os.remove("/tmp/chnlist_1")
		os.remove("/tmp/chnlist_2")
		os.remove("/tmp/chnlist_3")
		os.remove("/tmp/cdn_tmp")
		os.remove("/tmp/chnlist_tmp")
	end
end

if (enable_custom_url) == 0 then
	local version = curl(url_main .. "/version1")
	if version and tonumber(version) ~= 0 then
		if tonumber(gfwlist_update) == 1 then
			log("开始更新gfwlist...")
			local gfwlist = luci.sys.exec("echo -n $(echo '" .. version .. "' | sed -n 1p)")
			local new_version = luci.sys.exec("echo -n $(echo '" .. gfwlist .. "' | awk '{print $1}')")
			local new_md5 = luci.sys.exec("echo -n $(echo '" .. gfwlist .. "' | awk '{print $3}')")
			if new_version ~= "" and new_md5 ~= "" then
				local old_version = ucic:get_first(name, 'global_rules', "gfwlist_version", "nil")
				local old_md5 = luci.sys.exec("echo -n $(md5sum " .. rule_path .. "/gfwlist.conf | awk '{print $1}')")
				if old_md5 ~= new_md5 or old_version ~= new_version then
					curl(url_main .. "/gfwlist.conf", "/tmp/gfwlist_tmp")
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

		if tonumber(chnroute_update) == 1 and tonumber(enable_custom_url) == 0 then
			log("开始更新chnroute...")
			local chnroute = luci.sys.exec("echo -n $(echo '" .. version .. "' | sed -n 2p)")
			local new_version = luci.sys.exec("echo -n $(echo '" .. chnroute .. "' | awk '{print $1}')")
			local new_md5 = luci.sys.exec("echo -n $(echo '" .. chnroute .. "' | awk '{print $3}')")
			if new_version ~= "" and new_md5 ~= "" then
				local old_version = ucic:get_first(name, 'global_rules', "chnroute_version", "nil")
				local old_md5 = luci.sys.exec("echo -n $(md5sum " .. rule_path .. "/chnroute | awk '{print $1}')")
				if old_md5 ~= new_md5 or old_version ~= new_version then
					curl(url_main .. "/chnroute.txt", "/tmp/chnroute_tmp")
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
			log("开始更新chnlist...")
			local chnlist = luci.sys.exec("echo -n $(echo '" .. version .. "' | sed -n 4p)")
			local new_version = luci.sys.exec("echo -n $(echo '" .. chnlist .. "' | awk '{print $1}')")
			local new_md5 = luci.sys.exec("echo -n $(echo '" .. chnlist .. "' | awk '{print $3}')")
			if new_version ~= "" and new_md5 ~= "" then
				local old_version = ucic:get_first(name, 'global_rules', "chnlist_version", "nil")
				local old_md5 = luci.sys.exec("echo -n $(md5sum " .. rule_path .. "/chnlist | awk '{print $1}')")
				if old_md5 ~= new_md5 or old_version ~= new_version then
					curl(url_main .. "/cdn.txt", "/tmp/chnlist_tmp")
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
	else
		log("版本信息下载失败！")
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
log("规则脚本执行完毕...")