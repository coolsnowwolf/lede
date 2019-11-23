--[[
# Copyright (c) 2014-2016, latyas <latyas@gmail.com>
# Edit by Twinzo <1282055288@qq.com>
]]--

module("luci.controller.dogcom", package.seeall)

function index()
	
	if not nixio.fs.access("/etc/config/dogcom") then
		return
	end
	entry({"admin", "services", "dogcom"},alias("admin", "services", "dogcom", "dogcom"),_("DROCM客户端")).dependent = true
	entry({"admin", "services", "dogcom", "dogcom"}, cbi("dogcom/dogcom"),_("通用设置"),10).leaf = true
	entry({"admin", "services", "dogcom", "dogcom_proxy"}, cbi("dogcom/dogcom_proxy"),_("防代理设置"),30).leaf = true
	entry({"admin","services","dogcom","patch"},call("pppoe_patch")).leaf=true
	entry({"admin","services","dogcom","unpatch"},call("pppoe_unpatch")).leaf=true
end
function pppoe_patch()
	local set =luci.http.formvalue("set")
	local icount =0
	
	if set == "patch" then
		old_dogcom = luci.sys.exec("cat /lib/netifd/proto/ppp.sh | grep '#added by dogcom!' | wc -l")
		backup =luci.sys.call("cp -f /lib/netifd/proto/ppp.sh /lib/netifd/proto/ppp.sh_bak 2>/dev/null")
		if tonumber(old_dogcom) == 0 then 
			if backup == 0 then
				unpatach_cmd = "sed -i '/#added by dogcom/d' /lib/netifd/proto/ppp.sh"
				sret=luci.sys.call(unpatach_cmd .. " 2>/dev/null")
				icount = luci.sys.exec("cat /lib/netifd/proto/ppp.sh | wc -l")
				if  sret== 0 and tonumber(icount) ~= 0 then
					oldcount=luci.sys.exec("cat /lib/netifd/proto/ppp.sh_bak | wc -l")
					if tonumber(oldcount) ~= 0  then
						luci.sys.exec("sed -i '/proto_run_command/i username=`echo -e \"$username\"`  #added by dogcom!' /lib/netifd/proto/ppp.sh")
						luci.sys.exec("sed -i '/proto_run_command/i password=`echo -e \"$password\"`  #added by dogcom!' /lib/netifd/proto/ppp.sh")			
						retstring="1"
					else
						retstring ="0"
					end
				else
					retstring ="0"
				end
			end
		else
			retstring = "-1"
		end
	luci.sys.exec("rm -f /lib/netifd/proto/ppp.sh_bak")
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ ret=retstring })
end

function pppoe_unpatch()
	local set =luci.http.formvalue("set")
	local icount =0
	
	if set == "unpatch" then
		backup =luci.sys.call("cp -f /lib/netifd/proto/ppp.sh /lib/netifd/proto/ppp.sh_bak 2>/dev/null")
		if backup == 0 then
			unpatach_cmd = "sed -i '/#added by dogcom/d' /lib/netifd/proto/ppp.sh"
			sret=luci.sys.call(unpatach_cmd .. " 2>/dev/null")
			icount = luci.sys.exec("cat /lib/netifd/proto/ppp.sh | wc -l")
			if  sret== 0 and tonumber(icount) ~= 0 then
				oldcount=luci.sys.exec("cat /lib/netifd/proto/ppp.sh_bak | wc -l")
				if tonumber(icount) == tonumber(oldcount) then
					retstring ="-1"
				elseif tonumber(oldcount) > tonumber(icount) then
					retstring ="1"
				end
			else
				retstring ="0"
			end
		end
	luci.sys.exec("rm -f /lib/netifd/proto/ppp.sh_bak")
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ ret=retstring })
end