module("luci.controller.chinadns",package.seeall)
function index()
if not nixio.fs.access("/etc/config/chinadns")then
return
end
entry({"admin","services","ChinaDNS"},cbi("chinadns"),_("ChinaDNS"),35).dependent=true
entry({"admin","services","ChinaDNS","status"},call("act_status")).leaf=true
entry({"admin","services","ChinaDNS","refresh"},call("refresh_cmd")).leaf=true
end

function act_status()
  local set =luci.http.formvalue("set")
  local e={}
  e.running=luci.sys.call("pgrep -f chinadns >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end


function refresh_data()
	local set =luci.http.formvalue("set")
	local icount =0
	
	if set == "ip_data" then
		refresh_cmd="wget -O- 'http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest'  2>/dev/null| awk -F\\| '/CN\\|ipv4/ { printf(\"%s/%d\\n\", $4, 32-log($5)/log(2)) }' > /tmp/china_ssr.txt"
		
		--注意使用exec，使用luci.sys.call会出错，得不到标准输出
		sret=luci.sys.call(refresh_cmd .. " 2>/dev/null")
		icount = luci.sys.exec("cat /tmp/china_ssr.txt | wc -l")
		if  sret== 0 and tonumber(icount) > 1000 then
			oldcount=luci.sys.exec("cat `uci get chinadns.@chinadns[0].chnroute` | wc -l")
			if tonumber(icount) ~= tonumber(oldcount) then
				luci.sys.exec("cp -f /tmp/china_ssr.txt `uci get chinadns.@chinadns[0].chnroute`")
				retstring=tostring(tonumber(icount))
			else
				retstring ="0"
			end
		else
			retstring ="-1"
		end
		luci.sys.exec("rm -f /tmp/china_ssr.txt ")
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ ret=retstring })
end
