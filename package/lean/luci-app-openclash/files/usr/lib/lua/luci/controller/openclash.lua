module("luci.controller.openclash", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/openclash") then
		return
	end


	entry({"admin", "services", "openclash"},alias("admin", "services", "openclash", "client"), _("OpenClash"), 50).dependent = true
	entry({"admin", "services", "openclash", "client"},form("openclash/client"),_("Overviews"), 20).leaf = true
	entry({"admin", "services", "openclash", "status"},call("action_status")).leaf=true
	entry({"admin", "services", "openclash", "state"},call("action_state")).leaf=true
	entry({"admin", "services", "openclash", "startlog"},call("action_start")).leaf=true
	entry({"admin", "services", "openclash", "currentversion"},call("action_currentversion"))
	entry({"admin", "services", "openclash", "lastversion"},call("action_lastversion"))
	entry({"admin", "services", "openclash", "update"},call("action_update"))
	entry({"admin", "services", "openclash", "update_ma"},call("action_update_ma"))
	entry({"admin", "services", "openclash", "opupdate"},call("action_opupdate"))
	entry({"admin", "services", "openclash", "coreupdate"},call("action_coreupdate"))
	entry({"admin", "services", "openclash", "ping"}, call("act_ping"))
	entry({"admin", "services", "openclash", "download_game_rule"}, call("action_download_rule"))
	entry({"admin", "services", "openclash", "settings"},cbi("openclash/settings"),_("Global Settings"), 30).leaf = true
	entry({"admin", "services", "openclash", "servers"},cbi("openclash/servers"),_("Severs and Groups"), 40).leaf = true
	entry({"admin", "services", "openclash", "game-settings"},cbi("openclash/game-settings"),_("Game Rules and Groups"), 50).leaf = true
	entry({"admin", "services", "openclash", "game-rules-manage"},form("openclash/game-rules-manage"), nil).leaf = true
	entry({"admin", "services", "openclash", "config-subscribe"},cbi("openclash/config-subscribe"),_("Config Update"), 60).leaf = true
	entry({"admin", "services", "openclash", "servers-config"},cbi("openclash/servers-config"), nil).leaf = true
	entry({"admin", "services", "openclash", "groups-config"},cbi("openclash/groups-config"), nil).leaf = true
	entry({"admin", "services", "openclash", "proxy-provider-config"},cbi("openclash/proxy-provider-config"), nil).leaf = true
	entry({"admin", "services", "openclash", "config"},form("openclash/config"),_("Config Manage"), 70).leaf = true
	entry({"admin", "services", "openclash", "log"},form("openclash/log"),_("Logs"), 80).leaf = true

end
local fs = require "luci.openclash"

local function is_running()
	return luci.sys.call("pidof clash >/dev/null") == 0
end

local function is_web()
	return luci.sys.call("pidof clash >/dev/null") == 0
end

local function is_watchdog()
	return luci.sys.call("ps |grep openclash_watchdog.sh |grep -v grep >/dev/null") == 0
end

local function cn_port()
	return luci.sys.exec("uci get openclash.config.cn_port 2>/dev/null |tr -d '\n'")
end

local function mode()
	return luci.sys.exec("uci get openclash.config.en_mode 2>/dev/null")
end

local function ipdb()
	return os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/etc/openclash/Country.mmdb"))
end

local function lhie1()
	return os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/etc/openclash/lhie1.yaml"))
end

local function ConnersHua()
	return os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/etc/openclash/ConnersHua.yaml"))
end

local function ConnersHua_return()
	return os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/etc/openclash/ConnersHua_return.yaml"))
end

local function daip()
	local daip = luci.sys.exec("ifstatus lan 2>/dev/null |jsonfilter -e '@[\"ipv4-address\"][0].address' 2>/dev/null")
	if not daip or daip == "" then
		local daip = luci.sys.exec("uci get network.lan.ipaddr 2>/dev/null |awk -F '/' '{print $1}' 2>/dev/null |tr -d '\n'")
	end
	return daip
end

local function dase()
	return luci.sys.exec("uci get openclash.config.dashboard_password 2>/dev/null")
end

local function check_lastversion()
	luci.sys.exec("sh /usr/share/openclash/openclash_version.sh 2>/dev/null")
	return luci.sys.exec("sed -n '/^https:/,$p' /tmp/openclash_last_version 2>/dev/null")
end

local function check_currentversion()
	return luci.sys.exec("sed -n '/^data:image/,$p' /etc/openclash/openclash_version 2>/dev/null")
end

local function startlog()
	return luci.sys.exec("sed -n '$p' /tmp/openclash_start.log 2>/dev/null")
end

local function coremodel()
  local coremodel = luci.sys.exec("cat /proc/cpuinfo |grep 'cpu model' 2>/dev/null |awk -F ': ' '{print $2}' 2>/dev/null")
  local coremodel2 = luci.sys.exec("opkg status libc 2>/dev/null |grep 'Architecture' |awk -F ': ' '{print $2}' 2>/dev/null")
  if not coremodel or coremodel == "" then
     return coremodel2 .. "," .. coremodel2
  else
     return coremodel .. "," .. coremodel2
  end
end

local function corecv()
if not nixio.fs.access("/etc/openclash/clash") then
  return "0"
else
	return luci.sys.exec("/etc/openclash/clash -v 2>/dev/null |awk -F ' ' '{print $2}'")
end
end

local function corelv()
	local new = luci.sys.call(string.format("sh /usr/share/openclash/clash_version.sh"))
	local core_lv = luci.sys.exec("sed -n 1p /tmp/clash_last_version 2>/dev/null")
	return core_lv .. "," .. new
end

local function opcv()
	return luci.sys.exec("sed -n 1p /etc/openclash/openclash_version 2>/dev/null")
end

local function oplv()
	 local new = luci.sys.call(string.format("sh /usr/share/openclash/openclash_version.sh"))
	 local oplv = luci.sys.exec("sed -n 1p /tmp/openclash_last_version 2>/dev/null")
   return oplv .. "," .. new
end

local function opup()
   luci.sys.call("rm -rf /tmp/*_last_version 2>/dev/null && sh /usr/share/openclash/openclash_version.sh >/dev/null 2>&1")
   return luci.sys.call("sh /usr/share/openclash/openclash_update.sh >/dev/null 2>&1 &")
end

local function coreup()
   luci.sys.call("uci set openclash.config.enable=1 && uci commit openclash && rm -rf /tmp/*_last_version 2>/dev/null && sh /usr/share/openclash/clash_version.sh >/dev/null 2>&1")
   return luci.sys.call("sh /usr/share/openclash/openclash_core.sh >/dev/null 2>&1 &")
end

local function corever()
   return luci.sys.exec("uci get openclash.config.core_version 2>/dev/null")
end

local function upchecktime()
   local corecheck = os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/tmp/clash_last_version"))
   local opcheck
   if not corecheck or corecheck == "" then
      opcheck = os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/tmp/openclash_last_version"))
      if not opcheck or opcheck == "" then
         return "1"
      else
         return opcheck
      end
   else
      return corecheck
   end
end

function download_rule()
	local filename = luci.http.formvalue("filename")
	local rule_file_dir="/etc/openclash/game_rules/" .. filename
  local state = luci.sys.call(string.format('/usr/share/openclash/openclash_game_rule.sh "%s" >/dev/null 2>&1',filename))
  return state
end

function action_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
	  clash = is_running(),
		watchdog = is_watchdog(),
		daip = daip(),
		dase = dase(),
		web = is_web(),
		cn_port = cn_port(),
		mode = mode();
	})
end
function action_state()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		lhie1 = lhie1(),
		ConnersHua = ConnersHua(),
		ConnersHua_return = ConnersHua_return(),
		ipdb = ipdb();
	})
end

function action_lastversion()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
			lastversion = check_lastversion();
	})
end

function action_currentversion()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
			currentversion = check_currentversion();
	})
end

function action_start()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
			startlog = startlog();
	})
end

function action_update()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
			coremodel = coremodel(),
			corecv = corecv(),
			opcv = opcv(),
			corever = corever(),
			upchecktime = upchecktime(),
			corelv = corelv(),
			oplv = oplv();
	})
end

function action_update_ma()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
			oplv = oplv(),
			corever = corever();
	})
end

function action_opupdate()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
			opup = opup();
	})
end

function action_coreupdate()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
			coreup = coreup();
	})
end

function act_ping()
	local e={}
	e.index=luci.http.formvalue("index")
	e.ping=luci.sys.exec("ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*.[0-9]' | awk -F '=' '{print$2}'"%luci.http.formvalue("domain"))
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function action_download_rule()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		game_rule_download_status = download_rule();
	})
end
