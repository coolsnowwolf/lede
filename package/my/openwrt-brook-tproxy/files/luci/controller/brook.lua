-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Copyright (C) 2018 openwrt-brook-tproxy
-- Licensed to the public under the GNU General Public License v3.

module("luci.controller.brook", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/brook") then
		return
	end

	if nixio.fs.access("/usr/bin/brook") then
		entry({"admin", "services", "brook"},alias("admin", "services", "brook", "client"),_("Brook"), 10).dependent = true
		entry({"admin", "services", "brook", "client"},arcombine(cbi("brook/client"), cbi("brook/client-config")),_("Brook Tproxy"), 10).leaf = true
	else
		return
	end

	entry({"admin", "services", "brook", "status"},form("brook/status"),_("Status"), 20).leaf = true
	entry({"admin", "services", "brook", "refresh"}, call("refresh_data"))
	entry({"admin", "services", "brook", "custom"},form("brook/gfwlist-custom"),_("Custom List"), 30).leaf = true
end

function refresh_data()
	local set =luci.http.formvalue("set")
	local icount =0

	if set == "gfw_data" then
		if nixio.fs.access("/usr/bin/wget-ssl") then
			refresh_cmd="wget-ssl --no-check-certificate https://raw.githubusercontent.com/gfwlist/gfwlist/master/gfwlist.txt -O /tmp/gfw.b64"
		else
			refresh_cmd="wget -O /tmp/gfw.b64 http://iytc.net/tools/list.b64"
		end

		sret=luci.sys.call(refresh_cmd .. " 2>/dev/null")
		if sret== 0 then
			luci.sys.call("/usr/bin/brook-gfw")
			icount = luci.sys.exec("cat /tmp/gfwnew.txt | wc -l")
			if tonumber(icount)>1000 then
				oldcount=luci.sys.exec("cat /etc/dnsmasq.brook/gfw_list.conf | wc -l")
				if tonumber(icount) ~= tonumber(oldcount) then
					luci.sys.exec("cp -f /tmp/gfwnew.txt /etc/dnsmasq.brook/gfw_list.conf")
					retstring=tostring(math.ceil(tonumber(icount)/2))
				else
					retstring ="0"
				end
			else
				retstring ="-1"
			end
			luci.sys.exec("rm -f /tmp/gfwnew.txt ")
		else
			retstring ="-1"
		end
	elseif set == "ip_data" then
		refresh_cmd="wget -O- 'http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest'  2>/dev/null| awk -F\\| '/CN\\|ipv4/ { printf(\"%s/%d\\n\", $4, 32-log($5)/log(2)) }' > /tmp/china_ip.txt"
		sret=luci.sys.call(refresh_cmd)
		icount = luci.sys.exec("cat /tmp/china_ip.txt | wc -l")
		if  sret== 0 and tonumber(icount)>1000 then
			oldcount=luci.sys.exec("cat /etc/china_ip.txt | wc -l")
			if tonumber(icount) ~= tonumber(oldcount) then
				luci.sys.exec("cp -f /tmp/china_ip.txt /etc/china_ip.txt")
				retstring=tostring(tonumber(icount))
			else
				retstring ="0"
			end
		else
			retstring ="-1"
		end
		luci.sys.exec("rm -f /tmp/china_ip.txt ")
	else
		local need_process = 0
		if nixio.fs.access("/usr/bin/wget-ssl") then
			refresh_cmd="wget-ssl --no-check-certificate -O - https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt > /tmp/adnew.conf"
			need_process = 1
		else
			refresh_cmd="wget -O /tmp/ad.conf http://iytc.net/tools/ad.conf"
		end

		sret=luci.sys.call(refresh_cmd .. " 2>/dev/null")
		if sret== 0 then
			if need_process == 1 then
				luci.sys.call("/usr/bin/brook-ad")
			end
			icount = luci.sys.exec("cat /tmp/ad.conf | wc -l")
			if tonumber(icount)>1000 then
				if nixio.fs.access("/etc/dnsmasq.brook/ad.conf") then
					oldcount=luci.sys.exec("cat /etc/dnsmasq.brook/ad.conf | wc -l")
				else
					oldcount=0
				end
				if tonumber(icount) ~= tonumber(oldcount) then
					luci.sys.exec("cp -f /tmp/ad.conf /etc/dnsmasq.brook/ad.conf")
					retstring=tostring(math.ceil(tonumber(icount)))
					if oldcount==0 then
						luci.sys.call("/etc/init.d/dnsmasq restart")
					end
				else
					retstring ="0"
				end
			else
				retstring ="-1"
			end
			luci.sys.exec("rm -f /tmp/ad.conf ")
		else
			retstring ="-1"
		end
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ ret=retstring ,retcount=icount })
end