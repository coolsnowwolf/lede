module("luci.controller.openclash", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/openclash") then
		return
	end

	local page
	
	page = entry({"admin", "services", "openclash"}, alias("admin", "services", "openclash", "client"), _("OpenClash"), 50)
	page.dependent = true
	page.acl_depends = { "luci-app-openclash" }
	entry({"admin", "services", "openclash", "client"},cbi("openclash/client"),_("Overviews"), 20).leaf = true
	entry({"admin", "services", "openclash", "status"},call("action_status")).leaf=true
	entry({"admin", "services", "openclash", "state"},call("action_state")).leaf=true
	entry({"admin", "services", "openclash", "startlog"},call("action_start")).leaf=true
	entry({"admin", "services", "openclash", "refresh_log"},call("action_refresh_log"))
	entry({"admin", "services", "openclash", "del_log"},call("action_del_log"))
	entry({"admin", "services", "openclash", "close_all_connection"},call("action_close_all_connection"))
	entry({"admin", "services", "openclash", "restore_history"},call("action_restore_history"))
	entry({"admin", "services", "openclash", "get_history"},call("action_get_history"))
	entry({"admin", "services", "openclash", "reload_firewall"},call("action_reload_firewall"))
	entry({"admin", "services", "openclash", "update_subscribe"},call("action_update_subscribe"))
	entry({"admin", "services", "openclash", "update_other_rules"},call("action_update_other_rules"))
	entry({"admin", "services", "openclash", "update_geoip"},call("action_update_geoip"))
	entry({"admin", "services", "openclash", "currentversion"},call("action_currentversion"))
	entry({"admin", "services", "openclash", "lastversion"},call("action_lastversion"))
	entry({"admin", "services", "openclash", "update"},call("action_update"))
	entry({"admin", "services", "openclash", "update_ma"},call("action_update_ma"))
	entry({"admin", "services", "openclash", "opupdate"},call("action_opupdate"))
	entry({"admin", "services", "openclash", "coreupdate"},call("action_coreupdate"))
	entry({"admin", "services", "openclash", "coretunupdate"},call("action_core_tun_update"))
	entry({"admin", "services", "openclash", "coregameupdate"},call("action_core_game_update"))
	entry({"admin", "services", "openclash", "ping"}, call("act_ping"))
	entry({"admin", "services", "openclash", "download_rule"}, call("action_download_rule"))
	entry({"admin", "services", "openclash", "restore"}, call("action_restore_config"))
	entry({"admin", "services", "openclash", "one_key_update"}, call("action_one_key_update"))
	entry({"admin", "services", "openclash", "one_key_update_check"}, call("action_one_key_update_check"))
	entry({"admin", "services", "openclash", "switch_mode"}, call("action_switch_mode"))
	entry({"admin", "services", "openclash", "op_mode"}, call("action_op_mode"))
	entry({"admin", "services", "openclash", "settings"},cbi("openclash/settings"),_("Global Settings"), 30).leaf = true
	entry({"admin", "services", "openclash", "servers"},cbi("openclash/servers"),_("Servers and Groups"), 40).leaf = true
	entry({"admin", "services", "openclash", "other-rules-edit"},cbi("openclash/other-rules-edit"), nil).leaf = true
	entry({"admin", "services", "openclash", "rule-providers-settings"},cbi("openclash/rule-providers-settings"),_("Rule Providers and Groups"), 50).leaf = true
	entry({"admin", "services", "openclash", "game-rules-manage"},form("openclash/game-rules-manage"), nil).leaf = true
	entry({"admin", "services", "openclash", "rule-providers-manage"},form("openclash/rule-providers-manage"), nil).leaf = true
	entry({"admin", "services", "openclash", "proxy-provider-file-manage"},form("openclash/proxy-provider-file-manage"), nil).leaf = true
	entry({"admin", "services", "openclash", "rule-providers-file-manage"},form("openclash/rule-providers-file-manage"), nil).leaf = true
	entry({"admin", "services", "openclash", "config-subscribe"},cbi("openclash/config-subscribe"),_("Config Update"), 60).leaf = true
	entry({"admin", "services", "openclash", "config-subscribe-edit"},cbi("openclash/config-subscribe-edit"), nil).leaf = true
	entry({"admin", "services", "openclash", "servers-config"},cbi("openclash/servers-config"), nil).leaf = true
	entry({"admin", "services", "openclash", "groups-config"},cbi("openclash/groups-config"), nil).leaf = true
	entry({"admin", "services", "openclash", "proxy-provider-config"},cbi("openclash/proxy-provider-config"), nil).leaf = true
	entry({"admin", "services", "openclash", "rule-providers-config"},cbi("openclash/rule-providers-config"), nil).leaf = true
	entry({"admin", "services", "openclash", "config"},form("openclash/config"),_("Config Manage"), 70).leaf = true
	entry({"admin", "services", "openclash", "log"},cbi("openclash/log"),_("Server Logs"), 80).leaf = true

end
local fs = require "luci.openclash"

local core_path_mode = luci.sys.exec("uci get openclash.config.small_flash_memory 2>/dev/null |tr -d '\n'")
if core_path_mode ~= "1" then
	dev_core_path="/etc/openclash/core/clash"
	tun_core_path="/etc/openclash/core/clash_tun"
	game_core_path="/etc/openclash/core/clash_game"
else
	dev_core_path="/tmp/etc/openclash/core/clash"
	tun_core_path="/tmp/etc/openclash/core/clash_tun"
	game_core_path="/tmp/etc/openclash/core/clash_game"
end

local function is_running()
	return luci.sys.call("pidof clash >/dev/null") == 0
end

local function is_web()
	return luci.sys.call("pidof clash >/dev/null") == 0
end

local function restricted_mode()
	return luci.sys.exec("uci get openclash.config.restricted_mode 2>/dev/null |tr -d '\n'")
end

local function is_watchdog()
	local ps_version = luci.sys.exec("ps --version 2>&1 |grep -c procps-ng |tr -d '\n'")
	if ps_version == "0" then
		return luci.sys.call("ps |grep openclash_watchdog.sh |grep -v grep >/dev/null") == 0
	else
		return luci.sys.call("ps -ef |grep openclash_watchdog.sh |grep -v grep >/dev/null") == 0
	end
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
	return os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/usr/share/openclash/res/lhie1.yaml"))
end

local function ConnersHua()
	return os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/usr/share/openclash/res/ConnersHua.yaml"))
end

local function ConnersHua_return()
	return os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/usr/share/openclash/res/ConnersHua_return.yaml"))
end

local function chnroute()
	return os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/etc/openclash/rule_provider/ChinaIP.yaml"))
end

local function daip()
	local daip = luci.sys.exec("ifstatus lan 2>/dev/null |jsonfilter -e '@[\"ipv4-address\"][0].address' 2>/dev/null")
	if not daip or daip == "" then
		local daip = luci.sys.exec("uci get network.lan.ipaddr 2>/dev/null |awk -F '/' '{print $1}' 2>/dev/null |tr -d '\n'")
	end
	return daip
end

local function uh_port()
	local uh_port = luci.sys.exec("uci get uhttpd.main.listen_http |awk -F ':' '{print $NF}'")
	if uh_port ~= "80" then
		return ":" .. uh_port
	end
end

local function dase()
	return luci.sys.exec("uci get openclash.config.dashboard_password 2>/dev/null")
end

local function check_lastversion()
	luci.sys.exec("sh /usr/share/openclash/openclash_version.sh 2>/dev/null")
	return luci.sys.exec("sed -n '/^https:/,$p' /tmp/openclash_last_version 2>/dev/null")
end

local function check_currentversion()
	return luci.sys.exec("sed -n '/^data:image/,$p' /usr/share/openclash/res/openclash_version 2>/dev/null")
end

local function startlog()
	return luci.sys.exec("sed -n '$p' /tmp/openclash_start.log 2>/dev/null")
end

local function coremodel()
  local coremodel = luci.sys.exec("cat /usr/lib/os-release 2>/dev/null |grep OPENWRT_ARCH 2>/dev/null |awk -F '\"' '{print $2}' 2>/dev/null")
  local coremodel2 = luci.sys.exec("opkg status libc 2>/dev/null |grep 'Architecture' |awk -F ': ' '{print $2}' 2>/dev/null")
  if not coremodel or coremodel == "" then
     return coremodel2 .. "," .. coremodel2
  else
     return coremodel .. "," .. coremodel2
  end
end

local function corecv()
if not nixio.fs.access(dev_core_path) then
  return "0"
else
	return luci.sys.exec(string.format("%s -v 2>/dev/null |awk -F ' ' '{print $2}'",dev_core_path))
end
end

local function coretuncv()
if not nixio.fs.access(tun_core_path) then
  return "0"
else
	return luci.sys.exec(string.format("%s -v 2>/dev/null |awk -F ' ' '{print $2}'",tun_core_path))
end
end

local function coregamecv()
if not nixio.fs.access(game_core_path) then
  return "0"
else
	return luci.sys.exec(string.format("%s -v 2>/dev/null |awk -F ' ' '{print $2}'",game_core_path))
end
end

local function corelv()
	luci.sys.call("sh /usr/share/openclash/clash_version.sh")
	local core_lv = luci.sys.exec("sed -n 1p /tmp/clash_last_version 2>/dev/null")
	local core_tun_lv = luci.sys.exec("sed -n 2p /tmp/clash_last_version 2>/dev/null")
	local core_game_lv = luci.sys.exec("sed -n 3p /tmp/clash_last_version 2>/dev/null")
	return core_lv .. "," .. core_tun_lv .. "," .. core_game_lv
end

local function opcv()
	return luci.sys.exec("sed -n 1p /usr/share/openclash/res/openclash_version 2>/dev/null")
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
   return luci.sys.call("/usr/share/openclash/openclash_core.sh >/dev/null 2>&1 &")
end

local function coretunup()
   luci.sys.call("uci set openclash.config.enable=1 && uci commit openclash && rm -rf /tmp/*_last_version 2>/dev/null && sh /usr/share/openclash/clash_version.sh >/dev/null 2>&1")
   return luci.sys.call("/usr/share/openclash/openclash_core.sh 'Tun' >/dev/null 2>&1 &")
end

local function coregameup()
   luci.sys.call("uci set openclash.config.enable=1 && uci commit openclash && rm -rf /tmp/*_last_version 2>/dev/null && sh /usr/share/openclash/clash_version.sh >/dev/null 2>&1")
   return luci.sys.call("/usr/share/openclash/openclash_core.sh 'Game' >/dev/null 2>&1 &")
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

local function historychecktime()
	local CONFIG_FILE = string.sub(luci.sys.exec("uci get openclash.config.config_path 2>/dev/null"), 1, -2)
	local CONFIG_NAME = fs.basename(CONFIG_FILE)
  local HISTORY_PATH = "/etc/openclash/history/" .. CONFIG_NAME
	if not nixio.fs.access(HISTORY_PATH) then
  	return "0"
	else
		return os.date("%Y-%m-%d %H:%M:%S",fs.mtime(HISTORY_PATH))
	end
end

function download_rule()
	local filename = luci.http.formvalue("filename")
  local state = luci.sys.call(string.format('/usr/share/openclash/openclash_download_rule_list.sh "%s" >/dev/null 2>&1',filename))
  return state
end

function action_restore_config()
	luci.sys.call("/etc/init.d/openclash stop >/dev/null 2>&1")
	luci.sys.call("cp '/usr/share/openclash/backup/openclash' '/etc/config/openclash' >/dev/null 2>&1 &")
	luci.sys.call("cp '/usr/share/openclash/backup/openclash_custom_rules.list' '/etc/openclash/custom/openclash_custom_rules.list' >/dev/null 2>&1 &")
	luci.sys.call("cp '/usr/share/openclash/backup/openclash_custom_rules_2.list' '/etc/openclash/custom/openclash_custom_rules_2.list' >/dev/null 2>&1 &")
	luci.sys.call("cp '/usr/share/openclash/backup/openclash_custom_fake_black.conf' '/etc/openclash/custom/openclash_custom_fake_black.conf' >/dev/null 2>&1 &")
	luci.sys.call("cp '/usr/share/openclash/backup/openclash_custom_hosts.list' '/etc/openclash/custom/openclash_custom_hosts.list' >/dev/null 2>&1 &")
	luci.sys.call("cp '/usr/share/openclash/backup/openclash_custom_domain_dns.list' '/etc/openclash/custom/openclash_custom_domain_dns.list' >/dev/null 2>&1 &")
end

function action_one_key_update()
  return luci.sys.call("sh /usr/share/openclash/openclash_update.sh 'one_key_update' >/dev/null 2>&1 &")
end

function action_one_key_update_check()
	luci.sys.call("rm -rf /tmp/*_last_version 2>/dev/null")
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		corever = corever(),
		corelv = corelv(),
		oplv = oplv();
	})
end

function action_op_mode()
	local op_mode = luci.sys.exec("uci get openclash.config.operation_mode 2>/dev/null |tr -d '\n'")
	luci.http.prepare_content("application/json")
	luci.http.write_json({
	  op_mode = op_mode;
	})
end

function action_switch_mode()
	local switch_mode = luci.sys.exec("uci get openclash.config.operation_mode 2>/dev/null |tr -d '\n'")
	if switch_mode == "redir-host" then
	   luci.sys.call("uci set openclash.config.operation_mode=fake-ip >/dev/null 2>&1 && uci commit openclash")
	else
	   luci.sys.call("uci set openclash.config.operation_mode=redir-host >/dev/null 2>&1 && uci commit openclash")
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({
	  switch_mode = switch_mode;
	})
end

function action_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
	  clash = is_running(),
		watchdog = is_watchdog(),
		daip = daip(),
		dase = dase(),
		uh_port = uh_port(),
		web = is_web(),
		cn_port = cn_port(),
		restricted_mode = restricted_mode(),
		mode = mode();
	})
end

function action_state()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		lhie1 = lhie1(),
		ConnersHua = ConnersHua(),
		ConnersHua_return = ConnersHua_return(),
		ipdb = ipdb(),
		historychecktime = historychecktime(),
		chnroute = chnroute();
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
			coretuncv = coretuncv(),
			coregamecv = coregamecv(),
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
			corelv = corelv(),
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

function action_core_tun_update()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
			coretunup = coretunup();
	})
end

function action_core_game_update()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
			coregameup = coregameup();
	})
end

function action_close_all_connection()
	return luci.sys.call("sh /usr/share/openclash/openclash_history_set.sh close_all_conection")
end

function action_restore_history()
	return luci.sys.call("sh /usr/share/openclash/openclash_history_set.sh")
end

function action_get_history()
	return luci.sys.call("sh /usr/share/openclash/openclash_history_get.sh")
end

function action_reload_firewall()
	return luci.sys.call("/etc/init.d/openclash reload")
end

function action_update_subscribe()
	fs.unlink("/tmp/Proxy_Group")
	return luci.sys.call("/usr/share/openclash/openclash.sh >/dev/null 2>&1")
end

function action_update_other_rules()
	return luci.sys.call("/usr/share/openclash/openclash_rule.sh >/dev/null 2>&1")
end

function action_update_geoip()
	return luci.sys.call("/usr/share/openclash/openclash_ipdb.sh >/dev/null 2>&1")
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
		rule_download_status = download_rule();
	})
end

function action_refresh_log()
	local logfile="/tmp/openclash.log"
	if not fs.access(logfile) then
		luci.http.write("")
		return
	end
	luci.http.prepare_content("text/plain; charset=utf-8")
	local f=io.open(logfile, "r+")
	f:seek("set")
	local a=f:read(2048000) or ""
	f:close()
	luci.http.write(a)
end

function action_del_log()
	luci.sys.exec("echo '' > /tmp/openclash.log")
	return
end