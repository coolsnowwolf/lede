
module("luci.controller.adbyby", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/adbyby") then
		return
	end
	
	entry({"admin", "services", "adbyby"}, alias("admin", "services", "adbyby", "base"),_("ADBYBY Plus +"), 9).dependent = true
	
	entry({"admin", "services", "adbyby", "base"}, cbi("adbyby/base"), _("Base Setting"), 10).leaf=true
	entry({"admin", "services", "adbyby", "advanced"}, cbi("adbyby/advanced"), _("Advance Setting"), 20).leaf=true
	entry({"admin", "services", "adbyby", "help"}, form("adbyby/help"), _("Plus+ Domain List"), 30).leaf=true
	entry({"admin", "services", "adbyby", "esc"}, form("adbyby/esc"), _("Bypass Domain List"), 40).leaf=true
	entry({"admin", "services", "adbyby", "black"}, form("adbyby/black"), _("Block Domain List"), 50).leaf=true
	entry({"admin", "services", "adbyby", "block"}, form("adbyby/block"), _("Block IP List"), 60).leaf=true
	entry({"admin", "services", "adbyby", "user"}, form("adbyby/user"), _("User-defined Rule"), 70).leaf=true
	
	entry({"admin", "services", "adbyby", "refresh"}, call("refresh_data"))
	entry({"admin","services","adbyby","run"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("pgrep adbyby >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end


function refresh_data()
local set =luci.http.formvalue("set")
local icount =0

if set == "rule_data" then
luci.sys.exec("/usr/share/adbyby/rule-update")
  icount = luci.sys.exec("/usr/share/adbyby/rule-count '/tmp/rules/'")
  
  if tonumber(icount)>0 then
    if nixio.fs.access("/usr/share/adbyby/rules/") then
      oldcount=luci.sys.exec("/usr/share/adbyby/rule-count '/usr/share/adbyby/rules/'")
    else
      oldcount=0
    end
  else
    retstring ="-1"
  end
  
  if tonumber(icount) ~= tonumber(oldcount) then
		luci.sys.exec("rm -rf /usr/share/adbyby/rules/* && cp -a /tmp/rules /usr/share/adbyby/")
		luci.sys.exec("/etc/init.d/dnsmasq reload")
		retstring=tostring(math.ceil(tonumber(icount)))
	else
		retstring ="0"
	end
else
refresh_cmd="wget-ssl -q --no-check-certificate -O - 'https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt' > /tmp/adnew.conf"
sret=luci.sys.call(refresh_cmd .. " 2>/dev/null")
if sret== 0 then
	luci.sys.call("/usr/share/adbyby/ad-update")
	icount = luci.sys.exec("cat /tmp/ad.conf | wc -l")
	if tonumber(icount)>0 then
	if nixio.fs.access("/usr/share/adbyby/dnsmasq.adblock") then
		oldcount=luci.sys.exec("cat /usr/share/adbyby/dnsmasq.adblock | wc -l")
	else
		oldcount=0
	end
	if tonumber(icount) ~= tonumber(oldcount) then
		luci.sys.exec("cp -f /tmp/ad.conf /usr/share/adbyby/dnsmasq.adblock")
		luci.sys.exec("cp -f /tmp/ad.conf /tmp/etc/dnsmasq-adbyby.d/adblock")
		luci.sys.exec("/etc/init.d/dnsmasq reload")
		retstring=tostring(math.ceil(tonumber(icount)))
	else
		retstring ="0"
	end
	else
	retstring ="-1"
	end
	luci.sys.exec("rm -f /tmp/ad.conf")
else
	retstring ="-1"
end
end
luci.http.prepare_content("application/json")
luci.http.write_json({ ret=retstring ,retcount=icount})
end