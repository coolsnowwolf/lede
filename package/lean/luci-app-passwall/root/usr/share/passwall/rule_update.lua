#!/usr/bin/lua

require 'nixio'
require 'luci.sys'
local luci = luci
local ucic = luci.model.uci.cursor()
local jsonc = require "luci.jsonc"
local name = 'passwall'
local api = require ("luci.passwall.api")
local arg1 = arg[1]

local rule_path = "/usr/share/" .. name .. "/rules"
local reboot = 0
local gfwlist_update = 0
local chnroute_update = 0
local chnroute6_update = 0
local chnlist_update = 0
local geoip_update = 0
local geosite_update = 0

-- match comments/title/whitelist/ip address/excluded_domain
local comment_pattern = "^[!\\[@]+"
local ip_pattern = "^%d+%.%d+%.%d+%.%d+"
local ip4_ipset_pattern = "^%d+%.%d+%.%d+%.%d+[%/][%d]+$"
local ip6_ipset_pattern = ":-[%x]+%:+[%x]-[%/][%d]+$"
local domain_pattern = "([%w%-%_]+%.[%w%.%-%_]+)[%/%*]*"
local excluded_domain = {"apple.com","sina.cn","sina.com.cn","baidu.com","byr.cn","jlike.com","weibo.com","zhongsou.com","youdao.com","sogou.com","so.com","soso.com","aliyun.com","taobao.com","jd.com","qq.com","bing.com"}

local gfwlist_url = ucic:get(name, "@global_rules[0]", "gfwlist_url") or {"https://fastly.jsdelivr.net/gh/Loyalsoldier/v2ray-rules-dat@release/gfw.txt"}
local chnroute_url = ucic:get(name, "@global_rules[0]", "chnroute_url") or {"https://ispip.clang.cn/all_cn.txt"}
local chnroute6_url =  ucic:get(name, "@global_rules[0]", "chnroute6_url") or {"https://ispip.clang.cn/all_cn_ipv6.txt"}
local chnlist_url = ucic:get(name, "@global_rules[0]", "chnlist_url") or {"https://fastly.jsdelivr.net/gh/felixonmars/dnsmasq-china-list/accelerated-domains.china.conf","https://fastly.jsdelivr.net/gh/felixonmars/dnsmasq-china-list/apple.china.conf","https://fastly.jsdelivr.net/gh/felixonmars/dnsmasq-china-list/google.china.conf"}
local geoip_api =  "https://api.github.com/repos/Loyalsoldier/v2ray-rules-dat/releases/latest"
local geosite_api =  "https://api.github.com/repos/Loyalsoldier/v2ray-rules-dat/releases/latest"
local asset_location = ucic:get_first(name, 'global_rules', "v2ray_location_asset", "/usr/share/v2ray/")
local use_nft = ucic:get(name, "@global_forwarding[0]", "use_nft") or "0"

local log = function(...)
	if arg1 then
		if arg1 == "log" then
			api.log(...)
		elseif arg1 == "print" then
			local result = os.date("%Y-%m-%d %H:%M:%S: ") .. table.concat({...}, " ")
			print(result)
		end
	end
end

local function gen_nftset(set_name, ip_type, tmp_file, input_file)
	f = io.open(input_file, "r")
	local element = f:read("*all")
	f:close()

	nft_file, err = io.open(tmp_file, "w")
	nft_file:write('#!/usr/sbin/nft -f\n')
	nft_file:write(string.format('define %s = {%s}\n', set_name, string.gsub(element, "%s*%c+", " timeout 3650d, ")))
	if luci.sys.call(string.format('nft "list set inet fw4 %s" >/dev/null 2>&1', set_name)) ~= 0 then
		nft_file:write(string.format('add set inet fw4 %s { type %s; flags interval, timeout; timeout 2d; gc-interval 2d; auto-merge; }\n', set_name, ip_type))
	end
	nft_file:write(string.format('add element inet fw4 %s $%s\n', set_name, set_name))
	nft_file:close()
	luci.sys.call(string.format('nft -f %s &>/dev/null',tmp_file))
	os.remove(tmp_file)
end

--gen cache for nftset from file
local function gen_cache(set_name, ip_type, input_file, output_file)
	local tmp_dir = "/tmp/"
	local tmp_file = output_file .. "_tmp"
	local tmp_set_name = set_name .. "_tmp"
	gen_nftset(tmp_set_name, ip_type, tmp_file, input_file)
	luci.sys.call("nft list set inet fw4 " ..tmp_set_name.. " | sed 's/" ..tmp_set_name.. "/" ..set_name.. "/g' | cat > " ..output_file)
	luci.sys.call("nft flush set inet fw4 " ..tmp_set_name)
	luci.sys.call("nft delete set inet fw4 " ..tmp_set_name)
end

-- curl
local function curl(url, file, valifile)
	local args = {
		"-skL", "-w %{http_code}", "--retry 3", "--connect-timeout 3"
	}
	if file then
		args[#args + 1] = "-o " .. file
	end
	if valifile then
		args[#args + 1] = "--dump-header " .. valifile
	end
	local return_code, result = api.curl_logic(url, nil, args)
	return tonumber(result)
end

--check excluded domain
local function check_excluded_domain(value)
	for k,v in ipairs(excluded_domain) do
		if value:find(v) then
			return true
		end
	end
end

local function line_count(file_path)
	local num = 0
	for _ in io.lines(file_path) do
		num = num + 1
	end
	return num;
end

local function non_file_check(file_path, vali_file)
	if nixio.fs.readfile(file_path, 10) then
		local remote_file_size = tonumber(luci.sys.exec("cat " .. vali_file .. " | grep -i 'Content-Length' | awk '{print $2}'"))
		local local_file_size = tonumber(nixio.fs.stat(file_path, "size"))
		if remote_file_size and local_file_size then
			if remote_file_size == local_file_size then
				return nil;
			else
				log("下载文件大小校验出错，原始文件大小" .. remote_file_size .. "B，下载文件大小：" .. local_file_size .. "B。")
				return true;
			end
		else
			return nil;
		end
	else
		log("下载文件读取出错。")
		return true;
	end
end

--fetch rule
local function fetch_rule(rule_name,rule_type,url,exclude_domain)
	local sret = 200
	local sret_tmp = 0
	local domains = {}
	local file_tmp = "/tmp/" ..rule_name.. "_tmp"
	local vali_file = "/tmp/" ..rule_name.. "_vali"
	local download_file_tmp = "/tmp/" ..rule_name.. "_dl"
	local unsort_file_tmp = "/tmp/" ..rule_name.. "_unsort"

	log(rule_name.. " 开始更新...")
	for k,v in ipairs(url) do
		sret_tmp = curl(v, download_file_tmp..k, vali_file..k)
		if sret_tmp == 200 and non_file_check(download_file_tmp..k, vali_file..k) then
			log(rule_name.. " 第" ..k.. "条规则:" ..v.. "下载文件过程出错，尝试重新下载。")
			os.remove(download_file_tmp..k)
			os.remove(vali_file..k)
			sret_tmp = curl(v, download_file_tmp..k, vali_file..k)
			if sret_tmp == 200 and non_file_check(download_file_tmp..k, vali_file..k) then
				sret = 0
				sret_tmp = 0
				log(rule_name.. " 第" ..k.. "条规则:" ..v.. "下载文件过程出错，请检查网络或下载链接后重试！")
			end
		end

		if sret_tmp == 200 then
			if rule_name == "gfwlist" then
				local domains = {}
				local gfwlist = io.open(download_file_tmp..k, "r")
				local decode = api.base64Decode(gfwlist:read("*all"))
				gfwlist:close()

				gfwlist = io.open(download_file_tmp..k, "w")
				gfwlist:write(decode)
				gfwlist:close()
			end

			if rule_type == "domain" and exclude_domain == true then
				for line in io.lines(download_file_tmp..k) do
					if not (string.find(line, comment_pattern) or string.find(line, ip_pattern) or check_excluded_domain(line)) then
						local start, finish, match = string.find(line, domain_pattern)
						if (start) then
							domains[match] = true
						end
					end
				end

			elseif rule_type == "domain" then
				for line in io.lines(download_file_tmp..k) do
					if not (string.find(line, comment_pattern) or string.find(line, ip_pattern)) then
						local start, finish, match = string.find(line, domain_pattern)
						if (start) then
							domains[match] = true
						end
					end
				end

			elseif rule_type == "ip4" then
				local out = io.open(unsort_file_tmp, "a")
				for line in io.lines(download_file_tmp..k) do
					local start, finish, match = string.find(line, ip4_ipset_pattern)
					if (start) then
						out:write(string.format("%s\n", line))
					end
				end
				out:close()

			elseif rule_type == "ip6" then
				local out = io.open(unsort_file_tmp, "a")
				for line in io.lines(download_file_tmp..k) do
					local start, finish, match = string.find(line, ip6_ipset_pattern)
					if (start) then
						out:write(string.format("%s\n", line))
					end
				end
				out:close()

			end
		else
			sret = 0
			log(rule_name.. " 第" ..k.. "条规则:" ..v.. "下载失败，请检查网络或下载链接后重试！")
		end
		os.remove(download_file_tmp..k)
		os.remove(vali_file..k)
	end

	if sret == 200 then
		if rule_type == "domain" then
			local out = io.open(unsort_file_tmp, "w")
			for k,v in pairs(domains) do
				out:write(string.format("%s\n", k))
			end
			out:close()
		end
		luci.sys.call("cat " ..unsort_file_tmp.. " | sort -u > "..file_tmp)
		os.remove(unsort_file_tmp)

		local old_md5 = luci.sys.exec("echo -n $(md5sum " .. rule_path .. "/" ..rule_name.. " | awk '{print $1}')")
		local new_md5 = luci.sys.exec("echo -n $([ -f '" ..file_tmp.. "' ] && md5sum " ..file_tmp.." | awk '{print $1}')")
		if old_md5 ~= new_md5 then
			local count = line_count(file_tmp)
			if use_nft == "1" and (rule_type == "ip6" or rule_type == "ip4") then
				local set_name = "passwall_" ..rule_name
				local output_file = file_tmp.. ".nft"
				if rule_type == "ip4" then
					gen_cache(set_name, "ipv4_addr", file_tmp, output_file)
				elseif rule_type == "ip6" then
					gen_cache(set_name, "ipv6_addr", file_tmp, output_file)
				end
				luci.sys.exec(string.format('mv -f %s %s', output_file, rule_path .. "/" ..rule_name.. ".nft"))
				os.remove(output_file)
			end
			luci.sys.exec("mv -f "..file_tmp .. " " ..rule_path .. "/" ..rule_name)
			reboot = 1
			log(rule_name.. " 更新成功，总规则数 " ..count.. " 条。")
		else
			log(rule_name.. " 版本一致，无需更新。")
		end
	else
		log(rule_name.. " 文件下载失败！")
	end
	os.remove(file_tmp)
	return 0
end

local function fetch_gfwlist()
	fetch_rule("gfwlist","domain",gfwlist_url,true)
end

local function fetch_chnroute()
	fetch_rule("chnroute","ip4",chnroute_url,false)
end

local function fetch_chnroute6()
	fetch_rule("chnroute6","ip6",chnroute6_url,false)
end

local function fetch_chnlist()
	fetch_rule("chnlist","domain",chnlist_url,false)
end

--获取geoip
local function fetch_geoip()
	--请求geoip
	xpcall(function()
		local return_code, content = api.curl_logic(geoip_api)
		local json = jsonc.parse(content)
		if json.tag_name and json.assets then
			for _, v in ipairs(json.assets) do
				if v.name and v.name == "geoip.dat.sha256sum" then
					local sret = curl(v.browser_download_url, "/tmp/geoip.dat.sha256sum")
					if sret == 200 then
						local f = io.open("/tmp/geoip.dat.sha256sum", "r")
						local content = f:read()
						f:close()
						f = io.open("/tmp/geoip.dat.sha256sum", "w")
						f:write(content:gsub("geoip.dat", "/tmp/geoip.dat"), "")
						f:close()

						if nixio.fs.access(asset_location .. "geoip.dat") then
							luci.sys.call(string.format("cp -f %s %s", asset_location .. "geoip.dat", "/tmp/geoip.dat"))
							if luci.sys.call('sha256sum -c /tmp/geoip.dat.sha256sum > /dev/null 2>&1') == 0 then
								log("geoip 版本一致，无需更新。")
								return 1
							end
						end
						for _2, v2 in ipairs(json.assets) do
							if v2.name and v2.name == "geoip.dat" then
								sret = curl(v2.browser_download_url, "/tmp/geoip.dat")
								if luci.sys.call('sha256sum -c /tmp/geoip.dat.sha256sum > /dev/null 2>&1') == 0 then
									luci.sys.call(string.format("mkdir -p %s && cp -f %s %s", asset_location, "/tmp/geoip.dat", asset_location .. "geoip.dat"))
									reboot = 1
									log("geoip 更新成功。")
									return 1
								else
									log("geoip 更新失败，请稍后再试。")
								end
								break
							end
						end
					end
					break
				end
			end
		end
	end,
	function(e)
	end)

	return 0
end

--获取geosite
local function fetch_geosite()
	--请求geosite
	xpcall(function()
		local return_code, content = api.curl_logic(geosite_api)
		local json = jsonc.parse(content)
		if json.tag_name and json.assets then
			for _, v in ipairs(json.assets) do
				if v.name and v.name == "geosite.dat.sha256sum" then
					local sret = curl(v.browser_download_url, "/tmp/geosite.dat.sha256sum")
					if sret == 200 then
						local f = io.open("/tmp/geosite.dat.sha256sum", "r")
						local content = f:read()
						f:close()
						f = io.open("/tmp/geosite.dat.sha256sum", "w")
						f:write(content:gsub("geosite.dat", "/tmp/geosite.dat"), "")
						f:close()

						if nixio.fs.access(asset_location .. "geosite.dat") then
							luci.sys.call(string.format("cp -f %s %s", asset_location .. "geosite.dat", "/tmp/geosite.dat"))
							if luci.sys.call('sha256sum -c /tmp/geosite.dat.sha256sum > /dev/null 2>&1') == 0 then
								log("geosite 版本一致，无需更新。")
								return 1
							end
						end
						for _2, v2 in ipairs(json.assets) do
							if v2.name and v2.name == "geosite.dat" then
								sret = curl(v2.browser_download_url, "/tmp/geosite.dat")
								if luci.sys.call('sha256sum -c /tmp/geosite.dat.sha256sum > /dev/null 2>&1') == 0 then
									luci.sys.call(string.format("mkdir -p %s && cp -f %s %s", asset_location, "/tmp/geosite.dat", asset_location .. "geosite.dat"))
									reboot = 1
									log("geosite 更新成功。")
									return 1
								else
									log("geosite 更新失败，请稍后再试。")
								end
								break
							end
						end
					end
					break
				end
			end
		end
	end,
	function(e)
	end)

	return 0
end

if arg[2] then
	string.gsub(arg[2], '[^' .. "," .. ']+', function(w)
		if w == "gfwlist" then
			gfwlist_update = 1
		end
		if w == "chnroute" then
			chnroute_update = 1
		end
		if w == "chnroute6" then
			chnroute6_update = 1
		end
		if w == "chnlist" then
			chnlist_update = 1
		end
		if w == "geoip" then
			geoip_update = 1
		end
		if w == "geosite" then
			geosite_update = 1
		end
	end)
else
	gfwlist_update = ucic:get_first(name, 'global_rules', "gfwlist_update", 1)
	chnroute_update = ucic:get_first(name, 'global_rules', "chnroute_update", 1)
	chnroute6_update = ucic:get_first(name, 'global_rules', "chnroute6_update", 1)
	chnlist_update = ucic:get_first(name, 'global_rules', "chnlist_update", 1)
	geoip_update = ucic:get_first(name, 'global_rules', "geoip_update", 1)
	geosite_update = ucic:get_first(name, 'global_rules', "geosite_update", 1)
end
if gfwlist_update == 0 and chnroute_update == 0 and chnroute6_update == 0 and chnlist_update == 0 and geoip_update == 0 and geosite_update == 0 then
	os.exit(0)
end

log("开始更新规则...")
if tonumber(gfwlist_update) == 1 then
	xpcall(fetch_gfwlist,function(e)
		log(e)
		log(debug.traceback())
		log('更新gfwlist发生错误...')
	end)
end

if tonumber(chnroute_update) == 1 then
	xpcall(fetch_chnroute,function(e)
		log(e)
		log(debug.traceback())
		log('更新chnroute发生错误...')
	end)
end

if tonumber(chnroute6_update) == 1 then
	xpcall(fetch_chnroute6,function(e)
		log(e)
		log(debug.traceback())
		log('更新chnroute6发生错误...')
	end)
end

if tonumber(chnlist_update) == 1 then
	xpcall(fetch_chnlist,function(e)
		log(e)
		log(debug.traceback())
		log('更新chnlist发生错误...')
	end)
end

if tonumber(geoip_update) == 1 then
	log("geoip 开始更新...")
	local status = fetch_geoip()
	os.remove("/tmp/geoip.dat")
	os.remove("/tmp/geoip.dat.sha256sum")
end

if tonumber(geosite_update) == 1 then
	log("geosite 开始更新...")
	local status = fetch_geosite()
	os.remove("/tmp/geosite.dat")
	os.remove("/tmp/geosite.dat.sha256sum")
end

ucic:set(name, ucic:get_first(name, 'global_rules'), "gfwlist_update", gfwlist_update)
ucic:set(name, ucic:get_first(name, 'global_rules'), "chnroute_update", chnroute_update)
ucic:set(name, ucic:get_first(name, 'global_rules'), "chnroute6_update", chnroute6_update)
ucic:set(name, ucic:get_first(name, 'global_rules'), "chnlist_update", chnlist_update)
ucic:set(name, ucic:get_first(name, 'global_rules'), "geoip_update", geoip_update)
ucic:set(name, ucic:get_first(name, 'global_rules'), "geosite_update", geosite_update)
ucic:save(name)
luci.sys.call("uci commit " .. name)

if reboot == 1 then
	log("重启服务，应用新的规则。")
	if use_nft == "1" then
		luci.sys.call("sh /usr/share/" .. name .. "/nftables.sh flush_nftset > /dev/null 2>&1 &")
	else
		luci.sys.call("sh /usr/share/" .. name .. "/iptables.sh flush_ipset > /dev/null 2>&1 &")
	end
end
log("规则更新完毕...")
