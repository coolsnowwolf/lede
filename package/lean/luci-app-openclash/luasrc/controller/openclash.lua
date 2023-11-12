module("luci.controller.openclash", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/openclash") then
		return
	end

	local page
	
	page = entry({"admin", "services", "openclash"}, alias("admin", "services", "openclash", "client"), _("OpenClash"), 50)
	page.dependent = true
	page.acl_depends = { "luci-app-openclash" }
	entry({"admin", "services", "openclash", "client"},form("openclash/client"),_("Overviews"), 20).leaf = true
	entry({"admin", "services", "openclash", "status"},call("action_status")).leaf=true
	entry({"admin", "services", "openclash", "state"},call("action_state")).leaf=true
	entry({"admin", "services", "openclash", "startlog"},call("action_start")).leaf=true
	entry({"admin", "services", "openclash", "refresh_log"},call("action_refresh_log"))
	entry({"admin", "services", "openclash", "del_log"},call("action_del_log"))
	entry({"admin", "services", "openclash", "del_start_log"},call("action_del_start_log"))
	entry({"admin", "services", "openclash", "close_all_connection"},call("action_close_all_connection"))
	entry({"admin", "services", "openclash", "reload_firewall"},call("action_reload_firewall"))
	entry({"admin", "services", "openclash", "update_subscribe"},call("action_update_subscribe"))
	entry({"admin", "services", "openclash", "update_other_rules"},call("action_update_other_rules"))
	entry({"admin", "services", "openclash", "update_geoip"},call("action_update_geoip"))
	entry({"admin", "services", "openclash", "update_geosite"},call("action_update_geosite"))
	entry({"admin", "services", "openclash", "lastversion"},call("action_lastversion"))
	entry({"admin", "services", "openclash", "save_corever_branch"},call("action_save_corever_branch"))
	entry({"admin", "services", "openclash", "update"},call("action_update"))
	entry({"admin", "services", "openclash", "update_info"},call("action_update_info"))
	entry({"admin", "services", "openclash", "update_ma"},call("action_update_ma"))
	entry({"admin", "services", "openclash", "opupdate"},call("action_opupdate"))
	entry({"admin", "services", "openclash", "coreupdate"},call("action_coreupdate"))
	entry({"admin", "services", "openclash", "flush_fakeip_cache"}, call("action_flush_fakeip_cache"))
	entry({"admin", "services", "openclash", "download_rule"}, call("action_download_rule"))
	entry({"admin", "services", "openclash", "download_netflix_domains"}, call("action_download_netflix_domains"))
	entry({"admin", "services", "openclash", "download_disney_domains"}, call("action_download_disney_domains"))
	entry({"admin", "services", "openclash", "catch_netflix_domains"}, call("action_catch_netflix_domains"))
	entry({"admin", "services", "openclash", "write_netflix_domains"}, call("action_write_netflix_domains"))
	entry({"admin", "services", "openclash", "restore"}, call("action_restore_config"))
	entry({"admin", "services", "openclash", "backup"}, call("action_backup"))
	entry({"admin", "services", "openclash", "backup_ex_core"}, call("action_backup_ex_core"))
	entry({"admin", "services", "openclash", "backup_only_core"}, call("action_backup_only_core"))
	entry({"admin", "services", "openclash", "backup_only_config"}, call("action_backup_only_config"))
	entry({"admin", "services", "openclash", "backup_only_rule"}, call("action_backup_only_rule"))
	entry({"admin", "services", "openclash", "backup_only_proxy"}, call("action_backup_only_proxy"))
	entry({"admin", "services", "openclash", "remove_all_core"}, call("action_remove_all_core"))
	entry({"admin", "services", "openclash", "one_key_update"}, call("action_one_key_update"))
	entry({"admin", "services", "openclash", "one_key_update_check"}, call("action_one_key_update_check"))
	entry({"admin", "services", "openclash", "switch_mode"}, call("action_switch_mode"))
	entry({"admin", "services", "openclash", "op_mode"}, call("action_op_mode"))
	entry({"admin", "services", "openclash", "dler_info"}, call("action_dler_info"))
	entry({"admin", "services", "openclash", "dler_checkin"}, call("action_dler_checkin"))
	entry({"admin", "services", "openclash", "dler_logout"}, call("action_dler_logout"))
	entry({"admin", "services", "openclash", "dler_login"}, call("action_dler_login"))
	entry({"admin", "services", "openclash", "dler_login_info_save"}, call("action_dler_login_info_save"))
	entry({"admin", "services", "openclash", "sub_info_get"}, call("sub_info_get"))
	entry({"admin", "services", "openclash", "config_name"}, call("action_config_name"))
	entry({"admin", "services", "openclash", "switch_config"}, call("action_switch_config"))
	entry({"admin", "services", "openclash", "toolbar_show"}, call("action_toolbar_show"))
	entry({"admin", "services", "openclash", "toolbar_show_sys"}, call("action_toolbar_show_sys"))
	entry({"admin", "services", "openclash", "diag_connection"}, call("action_diag_connection"))
	entry({"admin", "services", "openclash", "diag_dns"}, call("action_diag_dns"))
	entry({"admin", "services", "openclash", "gen_debug_logs"}, call("action_gen_debug_logs"))
	entry({"admin", "services", "openclash", "log_level"}, call("action_log_level"))
	entry({"admin", "services", "openclash", "switch_log"}, call("action_switch_log"))
	entry({"admin", "services", "openclash", "rule_mode"}, call("action_rule_mode"))
	entry({"admin", "services", "openclash", "switch_rule_mode"}, call("action_switch_rule_mode"))
	entry({"admin", "services", "openclash", "switch_run_mode"}, call("action_switch_run_mode"))
	entry({"admin", "services", "openclash", "dashboard_type"}, call("action_dashboard_type"))
	entry({"admin", "services", "openclash", "switch_dashboard"}, call("action_switch_dashboard"))
	entry({"admin", "services", "openclash", "get_run_mode"}, call("action_get_run_mode"))
	entry({"admin", "services", "openclash", "create_file"}, call("create_file"))
	entry({"admin", "services", "openclash", "rename_file"}, call("rename_file"))
	entry({"admin", "services", "openclash", "manual_stream_unlock_test"}, call("manual_stream_unlock_test"))
	entry({"admin", "services", "openclash", "all_proxies_stream_test"}, call("all_proxies_stream_test"))
	entry({"admin", "services", "openclash", "set_subinfo_url"}, call("set_subinfo_url"))
	entry({"admin", "services", "openclash", "settings"},cbi("openclash/settings"),_("Plugin Settings"), 30).leaf = true
	entry({"admin", "services", "openclash", "config-overwrite"},cbi("openclash/config-overwrite"),_("Overwrite Settings"), 40).leaf = true
	entry({"admin", "services", "openclash", "servers"},cbi("openclash/servers"),_("Onekey Create"), 50).leaf = true
	entry({"admin", "services", "openclash", "other-rules-edit"},cbi("openclash/other-rules-edit"), nil).leaf = true
	entry({"admin", "services", "openclash", "custom-dns-edit"},cbi("openclash/custom-dns-edit"), nil).leaf = true
	entry({"admin", "services", "openclash", "other-file-edit"},cbi("openclash/other-file-edit"), nil).leaf = true
	entry({"admin", "services", "openclash", "rule-providers-settings"},cbi("openclash/rule-providers-settings"),_("Rule Providers Append"), 60).leaf = true
	entry({"admin", "services", "openclash", "game-rules-manage"},form("openclash/game-rules-manage"), nil).leaf = true
	entry({"admin", "services", "openclash", "rule-providers-manage"},form("openclash/rule-providers-manage"), nil).leaf = true
	entry({"admin", "services", "openclash", "proxy-provider-file-manage"},form("openclash/proxy-provider-file-manage"), nil).leaf = true
	entry({"admin", "services", "openclash", "rule-providers-file-manage"},form("openclash/rule-providers-file-manage"), nil).leaf = true
	entry({"admin", "services", "openclash", "game-rules-file-manage"},form("openclash/game-rules-file-manage"), nil).leaf = true
	entry({"admin", "services", "openclash", "config-subscribe"},cbi("openclash/config-subscribe"),_("Config Subscribe"), 70).leaf = true
	entry({"admin", "services", "openclash", "config-subscribe-edit"},cbi("openclash/config-subscribe-edit"), nil).leaf = true
	entry({"admin", "services", "openclash", "servers-config"},cbi("openclash/servers-config"), nil).leaf = true
	entry({"admin", "services", "openclash", "groups-config"},cbi("openclash/groups-config"), nil).leaf = true
	entry({"admin", "services", "openclash", "proxy-provider-config"},cbi("openclash/proxy-provider-config"), nil).leaf = true
	entry({"admin", "services", "openclash", "rule-providers-config"},cbi("openclash/rule-providers-config"), nil).leaf = true
	entry({"admin", "services", "openclash", "config"},form("openclash/config"),_("Config Manage"), 80).leaf = true
	entry({"admin", "services", "openclash", "log"},cbi("openclash/log"),_("Server Logs"), 90).leaf = true

end
local fs = require "luci.openclash"
local json = require "luci.jsonc"
local uci = require("luci.model.uci").cursor()
local datatype = require "luci.cbi.datatypes"
local opkg
if pcall(require, "luci.model.ipkg") then
	opkg = require "luci.model.ipkg"
end

local core_path_mode = uci:get("openclash", "config", "small_flash_memory")
if core_path_mode ~= "1" then
	dev_core_path="/etc/openclash/core/clash"
	tun_core_path="/etc/openclash/core/clash_tun"
	meta_core_path="/etc/openclash/core/clash_meta"
else
	dev_core_path="/tmp/etc/openclash/core/clash"
	tun_core_path="/tmp/etc/openclash/core/clash_tun"
	meta_core_path="/tmp/etc/openclash/core/clash_meta"
end

local function is_running()
	return luci.sys.call("pidof clash >/dev/null") == 0
end

local function is_web()
	return luci.sys.call("pidof clash >/dev/null") == 0
end

local function restricted_mode()
	return uci:get("openclash", "config", "restricted_mode")
end

local function is_watchdog()
	return process_status("openclash_watchdog.sh")
end

local function is_start()
	return process_status("/etc/init.d/openclash")
end

local function cn_port()
	return uci:get("openclash", "config", "cn_port")
end

local function mode()
	return uci:get("openclash", "config", "en_mode")
end

local function ipdb()
	return os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/etc/openclash/Country.mmdb"))
end

local function geosite()
	return os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/etc/openclash/GeoSite.dat"))
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
	return os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/etc/openclash/china_ip_route.ipset"))
end

local function chnroutev6()
	return os.date("%Y-%m-%d %H:%M:%S",fs.mtime("/etc/openclash/china_ip6_route.ipset"))
end

local function daip()
	local daip = luci.sys.exec("uci -q get network.lan.ipaddr |awk -F '/' '{print $1}' 2>/dev/null |tr -d '\n'")
	if not daip or daip == "" then
		daip = luci.sys.exec("ip address show $(uci -q -p /tmp/state get network.lan.ifname || uci -q -p /tmp/state get network.lan.device) | grep -w 'inet'  2>/dev/null |grep -Eo 'inet [0-9\.]+' | awk '{print $2}' | tr -d '\n'")
	end
	if not daip or daip == "" then
		daip = luci.sys.exec("ip addr show 2>/dev/null | grep -w 'inet' | grep 'global' | grep 'brd' | grep -Eo 'inet [0-9\.]+' | awk '{print $2}' | head -n 1 | tr -d '\n'")
	end
	return daip
end

local function dase()
	return uci:get("openclash", "config", "dashboard_password")
end

local function db_foward_domain()
	return uci:get("openclash", "config", "dashboard_forward_domain")
end

local function db_foward_port()
	return uci:get("openclash", "config", "dashboard_forward_port")
end

local function db_foward_ssl()
	return uci:get("openclash", "config", "dashboard_forward_ssl") or 0
end

local function check_lastversion()
	luci.sys.exec("bash /usr/share/openclash/openclash_version.sh 2>/dev/null")
	return luci.sys.exec("sed -n '/^https:/,$p' /tmp/openclash_last_version 2>/dev/null")
end

local function startlog()
	local info = ""
	local line_trans = ""
	if nixio.fs.access("/tmp/openclash_start.log") then
		info = luci.sys.exec("sed -n '$p' /tmp/openclash_start.log 2>/dev/null")
		line_trans = info
		if string.len(info) > 0 then
			if not string.find (info, "【") or not string.find (info, "】") then
				line_trans = trans_line_nolabel(info)
   			else
   				line_trans = trans_line(info)
   			end
   		end
	end
	return line_trans
end

local function coremodel()
	if opkg and opkg.info("libc") and opkg.info("libc")["libc"] then
		return opkg.info("libc")["libc"]["Architecture"]
	else
		return luci.sys.exec("rm -f /var/lock/opkg.lock && opkg status libc 2>/dev/null |grep 'Architecture' |awk -F ': ' '{print $2}' 2>/dev/null")
	end
end

local function corecv()
	if not nixio.fs.access(dev_core_path) then
		return "0"
	else
		return luci.sys.exec(string.format("%s -v 2>/dev/null |awk -F ' ' '{print $2}'", dev_core_path))
	end
end

local function coretuncv()
	if not nixio.fs.access(tun_core_path) then
		return "0"
	else
		return luci.sys.exec(string.format("%s -v 2>/dev/null |awk -F ' ' '{print $2}'", tun_core_path))
	end
end

local function coremetacv()
	if not nixio.fs.access(meta_core_path) then
		return "0"
	else
		return luci.sys.exec(string.format("%s -v 2>/dev/null |awk -F ' ' '{print $3}' |head -1", meta_core_path))
	end
end

local function corelv()
	luci.sys.call("bash /usr/share/openclash/clash_version.sh")
	local core_lv = luci.sys.exec("sed -n 1p /tmp/clash_last_version 2>/dev/null")
	local core_tun_lv = luci.sys.exec("sed -n 2p /tmp/clash_last_version 2>/dev/null")
	local core_meta_lv = luci.sys.exec("sed -n 3p /tmp/clash_last_version 2>/dev/null")
	return core_lv .. "," .. core_tun_lv .. "," .. core_meta_lv
end

local function opcv()
	if opkg and opkg.info("luci-app-openclash") and opkg.info("luci-app-openclash")["luci-app-openclash"] then
		return "v" .. opkg.info("luci-app-openclash")["luci-app-openclash"]["Version"]
	else
		return luci.sys.exec("rm -f /var/lock/opkg.lock && opkg status luci-app-openclash 2>/dev/null |grep 'Version' |awk -F 'Version: ' '{print \"v\"$2}'")
	end
end

local function oplv()
	local new = luci.sys.call(string.format("bash /usr/share/openclash/openclash_version.sh"))
	local oplv = luci.sys.exec("sed -n 1p /tmp/openclash_last_version 2>/dev/null")
	return oplv .. "," .. new
end

local function opup()
	luci.sys.call("rm -rf /tmp/*_last_version 2>/dev/null && bash /usr/share/openclash/openclash_version.sh >/dev/null 2>&1")
	return luci.sys.call("bash /usr/share/openclash/openclash_update.sh >/dev/null 2>&1 &")
end

local function coreup()
	uci:set("openclash", "config", "enable", "1")
	uci:commit("openclash")
	local type = luci.http.formvalue("core_type")
	luci.sys.call("rm -rf /tmp/*_last_version 2>/dev/null && bash /usr/share/openclash/clash_version.sh >/dev/null 2>&1")
	return luci.sys.call(string.format("/usr/share/openclash/openclash_core.sh '%s' >/dev/null 2>&1 &", type))
end

local function corever()
	return uci:get("openclash", "config", "core_version")
end

local function release_branch()
	return uci:get("openclash", "config", "release_branch")
end

local function save_corever_branch()
	if luci.http.formvalue("core_ver") then
		uci:set("openclash", "config", "core_version", luci.http.formvalue("core_ver"))
	end
	if luci.http.formvalue("release_branch") then
		uci:set("openclash", "config", "release_branch", luci.http.formvalue("release_branch"))
	end
	uci:commit("openclash")
	return "success"
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
	local CONFIG_FILE = uci:get("openclash", "config", "config_path")
	if not CONFIG_FILE then return "0" end
	local HISTORY_PATH_OLD = "/etc/openclash/history/" .. fs.filename(fs.basename(CONFIG_FILE))
	local HISTORY_PATH = "/etc/openclash/history/" .. fs.filename(fs.basename(CONFIG_FILE)) .. ".db"
	if not nixio.fs.access(HISTORY_PATH) and not nixio.fs.access(HISTORY_PATH_OLD) then
  	return "0"
	else
		return os.date("%Y-%m-%d %H:%M:%S",fs.mtime(HISTORY_PATH)) or os.date("%Y-%m-%d %H:%M:%S",fs.mtime(HISTORY_PATH_OLD))
	end
end

function download_rule()
	local filename = luci.http.formvalue("filename")
	local state = luci.sys.call(string.format('/usr/share/openclash/openclash_download_rule_list.sh "%s" >/dev/null 2>&1',filename))
	return state
end

function download_disney_domains()
	local state = luci.sys.call(string.format('/usr/share/openclash/openclash_download_rule_list.sh "%s" >/dev/null 2>&1',"disney_domains"))
	return state
end

function download_netflix_domains()
	local state = luci.sys.call(string.format('/usr/share/openclash/openclash_download_rule_list.sh "%s" >/dev/null 2>&1',"netflix_domains"))
	return state
end

function action_flush_fakeip_cache()
	local state = 0
	if is_running() then
		local daip = daip()
		local dase = dase() or ""
		local cn_port = cn_port()
		if not daip or not cn_port then return end
  	state = luci.sys.exec(string.format('curl -sL -m 3 -H "Content-Type: application/json" -H "Authorization: Bearer %s" -XPOST http://"%s":"%s"/cache/fakeip/flush', dase, daip, cn_port))
  end
  luci.http.prepare_content("application/json")
	luci.http.write_json({
		flush_status = state;
	})
end

function action_restore_config()
	uci:set("openclash", "config", "enable", "0")
	uci:commit("openclash")
	luci.sys.call("/etc/init.d/openclash stop >/dev/null 2>&1")
	luci.sys.call("cp '/usr/share/openclash/backup/openclash' '/etc/config/openclash' >/dev/null 2>&1 &")
	luci.sys.call("cp /usr/share/openclash/backup/openclash_custom* /etc/openclash/custom/ >/dev/null 2>&1 &")
	luci.sys.call("cp /usr/share/openclash/backup/openclash_force_sniffing* /etc/openclash/custom/ >/dev/null 2>&1 &")
	luci.sys.call("cp /usr/share/openclash/backup/openclash_sniffing* /etc/openclash/custom/ >/dev/null 2>&1 &")
	luci.sys.call("cp /usr/share/openclash/backup/yml_change.sh /usr/share/openclash/yml_change.sh >/dev/null 2>&1 &")
	luci.sys.call("rm -rf /etc/openclash/history/* >/dev/null 2>&1 &")
	luci.http.redirect(luci.dispatcher.build_url('admin/services/openclash/settings'))
end

function action_remove_all_core()
	luci.sys.call("rm -rf /etc/openclash/core/* >/dev/null 2>&1")
end

function action_one_key_update()
  return luci.sys.call("rm -rf /tmp/*_last_version 2>/dev/null && bash /usr/share/openclash/openclash_update.sh 'one_key_update' >/dev/null 2>&1 &")
end

local function dler_login_info_save()
	uci:set("openclash", "config", "dler_email", luci.http.formvalue("email"))
	uci:set("openclash", "config", "dler_passwd", luci.http.formvalue("passwd"))
	uci:set("openclash", "config", "dler_checkin", luci.http.formvalue("checkin"))
	uci:set("openclash", "config", "dler_checkin_interval", luci.http.formvalue("interval"))
	if tonumber(luci.http.formvalue("multiple")) > 50 then
		uci:set("openclash", "config", "dler_checkin_multiple", "50")
	elseif tonumber(luci.http.formvalue("multiple")) < 1 or not tonumber(luci.http.formvalue("multiple")) then
		uci:set("openclash", "config", "dler_checkin_multiple", "1")
	else
		uci:set("openclash", "config", "dler_checkin_multiple", luci.http.formvalue("multiple"))
	end
	uci:commit("openclash")
	return "success"
end

local function dler_login()
	local info, token, get_sub, sub_info, sub_key, sub_match
	local sub_path = "/tmp/dler_sub"
	local email = uci:get("openclash", "config", "dler_email")
	local passwd = uci:get("openclash", "config", "dler_passwd")
	if email and passwd then
		info = luci.sys.exec(string.format("curl -sL -H 'Content-Type: application/json' -d '{\"email\":\"%s\", \"passwd\":\"%s\"}' -X POST https://dler.cloud/api/v1/login", email, passwd))
		if info then
			info = json.parse(info)
		end
		if info and info.ret == 200 then
			token = info.data.token
			uci:set("openclash", "config", "dler_token", token)
			uci:commit("openclash")
			get_sub = string.format("curl -sL -H 'Content-Type: application/json' -d '{\"access_token\":\"%s\"}' -X POST https://dler.cloud/api/v1/managed/clash -o %s", token, sub_path)
			luci.sys.exec(get_sub)
			sub_info = fs.readfile(sub_path)
			if sub_info then
				sub_info = json.parse(sub_info)
			end
			if sub_info and sub_info.ret == 200 then
				sub_key = {"smart","ss","vmess","trojan"}
				for _,v in ipairs(sub_key) do
					while true do
						sub_match = false
						uci:foreach("openclash", "config_subscribe",
						function(s)
							if s.name == "Dler Cloud - " .. v and s.address == sub_info[v] then
			   				sub_match = true
							end
						end)
						if sub_match then break end
						luci.sys.exec(string.format('sid=$(uci -q add openclash config_subscribe) && uci -q set openclash."$sid".name="Dler Cloud - %s" && uci -q set openclash."$sid".address="%s"', v, sub_info[v]))
						uci:commit("openclash")
						break
					end
					luci.sys.exec(string.format('curl -sL -m 3 --retry 2 --user-agent "clash" "%s" -o "/etc/openclash/config/Dler Cloud - %s.yaml" >/dev/null 2>&1', sub_info[v], v))
				end
			end
			return info.ret
		else
			uci:delete("openclash", "config", "dler_token")
			uci:commit("openclash")
			fs.unlink(sub_path)
			fs.unlink("/tmp/dler_checkin")
			fs.unlink("/tmp/dler_info")
			if info and info.msg then
				return info.msg
			else
				return "login faild"
			end
		end
	else
		uci:delete("openclash", "config", "dler_token")
		uci:commit("openclash")
		fs.unlink(sub_path)
		fs.unlink("/tmp/dler_checkin")
		fs.unlink("/tmp/dler_info")
		return "email or passwd is wrong"
	end
end

local function dler_logout()
	local info, token
	local token = uci:get("openclash", "config", "dler_token")
	if token then
		info = luci.sys.exec(string.format("curl -sL -H 'Content-Type: application/json' -d '{\"access_token\":\"%s\"}' -X POST https://dler.cloud/api/v1/logout", token))
		if info then
			info = json.parse(info)
		end
		if info and info.ret == 200 then
			uci:delete("openclash", "config", "dler_token")
			uci:delete("openclash", "config", "dler_checkin")
			uci:delete("openclash", "config", "dler_checkin_interval")
			uci:delete("openclash", "config", "dler_checkin_multiple")
			uci:commit("openclash")
			fs.unlink("/tmp/dler_sub")
			fs.unlink("/tmp/dler_checkin")
			fs.unlink("/tmp/dler_info")
			return info.ret
		else
			if info and info.msg then
				return info.msg
			else
				return "logout faild"
			end
		end
	else
		return "logout faild"
	end
end

local function dler_info()
	local info, path, get_info
	local token = uci:get("openclash", "config", "dler_token")
	local email = uci:get("openclash", "config", "dler_email")
	local passwd = uci:get("openclash", "config", "dler_passwd")
	path = "/tmp/dler_info"
	if token and email and passwd then
		get_info = string.format("curl -sL -H 'Content-Type: application/json' -d '{\"email\":\"%s\", \"passwd\":\"%s\"}' -X POST https://dler.cloud/api/v1/information -o %s", email, passwd, path)
		if not nixio.fs.access(path) then
			luci.sys.exec(get_info)
		else
			if fs.readfile(path) == "" or not fs.readfile(path) then
				luci.sys.exec(get_info)
			else
				if (os.time() - fs.mtime(path) > 900) then
					luci.sys.exec(get_info)
				end
			end
		end
		info = fs.readfile(path)
		if info then
			info = json.parse(info)
		end
		if info and info.ret == 200 then
			return info.data
		else
			fs.unlink(path)
			luci.sys.exec(string.format("echo -e %s Dler Cloud Account Login Failed! Please Check And Try Again... >> /tmp/openclash.log", os.date("%Y-%m-%d %H:%M:%S")))
			return "error"
		end
	else
		return "error"
	end
end

local function dler_checkin()
	local info
	local path = "/tmp/dler_checkin"
	local token = uci:get("openclash", "config", "dler_token")
	local email = uci:get("openclash", "config", "dler_email")
	local passwd = uci:get("openclash", "config", "dler_passwd")
	local multiple = uci:get("openclash", "config", "dler_checkin_multiple") or 1
	if token and email and passwd then
		info = luci.sys.exec(string.format("curl -sL -H 'Content-Type: application/json' -d '{\"email\":\"%s\", \"passwd\":\"%s\", \"multiple\":\"%s\"}' -X POST https://dler.cloud/api/v1/checkin", email, passwd, multiple))
		if info then
			info = json.parse(info)
		end
		if info and info.ret == 200 then
			fs.unlink("/tmp/dler_info")
			fs.writefile(path, info)
			luci.sys.exec(string.format("echo -e %s Dler Cloud Checkin Successful, Result:【%s】 >> /tmp/openclash.log", os.date("%Y-%m-%d %H:%M:%S"), info.data.checkin))
			return info
		else
			if info and info.msg then
				luci.sys.exec(string.format("echo -e %s Dler Cloud Checkin Failed, Result:【%s】 >> /tmp/openclash.log", os.date("%Y-%m-%d %H:%M:%S"), info.msg))
			else
				luci.sys.exec(string.format("echo -e %s Dler Cloud Checkin Failed! Please Check And Try Again... >> /tmp/openclash.log",os.date("%Y-%m-%d %H:%M:%S")))
			end
			return info
		end
	else
		return "error"
	end
end

local function config_name()
	local e,a={}
	for t,o in ipairs(fs.glob("/etc/openclash/config/*"))do
		a=fs.stat(o)
		if a then
			e[t]={}
			e[t].name=fs.basename(o)
		end
	end
	return json.parse(json.stringify(e)) or e
end

local function config_path()
	if uci:get("openclash", "config", "config_path") then
		return string.sub(uci:get("openclash", "config", "config_path"), 23, -1)
	else
		 return ""
	end
end

function action_switch_config()
	uci:set("openclash", "config", "config_path", "/etc/openclash/config/"..luci.http.formvalue("config_name"))
	uci:commit("openclash")
end

function set_subinfo_url()
	local filename, url, info
	filename = luci.http.formvalue("filename")
	url = luci.http.formvalue("url")
	if not filename then
		info = "Oops: The config file name seems to be incorrect"
	end
	if url ~= "" and not string.find(url, "http") then
		info = "Oops: The url link format seems to be incorrect"
	end
	if not info then
		uci:foreach("openclash", "subscribe_info",
			function(s)
				if s.name == filename then
					if url == "" then
						uci:delete("openclash", s[".name"])
						uci:commit("openclash")
						info = "Delete success"
					else
						uci:set("openclash", s[".name"], "url", url)
						uci:commit("openclash")
						info = "Success"
					end
				end
			end
		)
		if not info then
			if url == "" then
				info = "Delete success"
			else
				uci:section("openclash", "subscribe_info", nil, {name = filename, url = url})
				uci:commit("openclash")
				info = "Success"
			end
		end
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		info = info;
	})
end

function sub_info_get()
	local filepath, filename, sub_url, sub_info, info, upload, download, total, expire, http_code, len, percent, day_left, day_expire
	local info_tb = {}
	filename = luci.http.formvalue("filename")
	sub_info = ""
	if filename and not is_start() then
		uci:foreach("openclash", "subscribe_info",
			function(s)
				if s.name == filename and s.url and string.find(s.url, "http") then
					string.gsub(s.url, '[^\n]+', function(w) table.insert(info_tb, w) end)
					sub_url = info_tb[1]
				end
			end
		)
		if not sub_url then
			uci:foreach("openclash", "config_subscribe",
				function(s)
					if s.name == filename and s.address and string.find(s.address, "http") then
						string.gsub(s.address, '[^\n]+', function(w) table.insert(info_tb, w) end)
						sub_url = info_tb[1]
					end
				end
			)
		end
		if not sub_url then
			sub_info = "No Sub Info Found"
		else
			info = luci.sys.exec(string.format("curl -sLI -X GET -m 10 -w 'http_code='%%{http_code} -H 'User-Agent: Clash' '%s'", sub_url))
			if not info or tonumber(string.sub(string.match(info, "http_code=%d+"), 11, -1)) ~= 200 then
				info = luci.sys.exec(string.format("curl -sLI -X GET -m 10 -w 'http_code='%%{http_code} -H 'User-Agent: Quantumultx' '%s'", sub_url))
			end
			if info then
				http_code=string.sub(string.match(info, "http_code=%d+"), 11, -1)
				if tonumber(http_code) == 200 then
					info = string.lower(info)
					if string.find(info, "subscription%-userinfo") then
						info = luci.sys.exec("echo '%s' |grep 'subscription-userinfo'" %info)
						upload = string.sub(string.match(info, "upload=%d+"), 8, -1) or nil
						download = string.sub(string.match(info, "download=%d+"), 10, -1) or nil
						total = tonumber(string.format("%.1f",string.sub(string.match(info, "total=%d+"), 7, -1))) or nil
						used = tonumber(string.format("%.1f",(upload + download))) or nil
						if string.match(info, "expire=%d+") then
							day_expire = tonumber(string.sub(string.match(info, "expire=%d+"), 8, -1)) or nil
						end
						expire = os.date("%Y-%m-%d", day_expire) or "null"
						if day_expire and os.time() <= day_expire then
							day_left = math.ceil((day_expire - os.time()) / (3600*24))
						elseif day_expire == nil then
							day_left = "null"
						else
							day_left = 0
						end
						
						if used and total and used < total then
							percent = string.format("%.1f",((total-used)/total)*100) or nil
						elseif used == nil or total == nil or total == 0 then
							percent = 100
						else
							percent = 0
						end
						total = fs.filesize(total) or "null"
						used = fs.filesize(used) or "null"
						sub_info = "Successful"
					else
						sub_info = "No Sub Info Found"
					end
				end
			end
		end
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		http_code = http_code,
		sub_info = sub_info,
		used = used,
		total = total,
		percent = percent,
		day_left = day_left,
		expire = expire;
	})
end

function action_rule_mode()
	local mode, info, core_type
	if is_running() then
		local daip = daip()
		local dase = dase() or ""
		local cn_port = cn_port()
		core_type = uci:get("openclash", "config", "core_type") or "Dev"
		if not daip or not cn_port then return end
		info = json.parse(luci.sys.exec(string.format('curl -sL -m 3 -H "Content-Type: application/json" -H "Authorization: Bearer %s" -XGET http://"%s":"%s"/configs', dase, daip, cn_port)))
		if info then
			mode = info["mode"]
		else
			mode = uci:get("openclash", "config", "proxy_mode") or "rule"
		end
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		mode = mode,
		core_type = core_type;
	})
end

function action_switch_rule_mode()
	local mode, info
	if is_running() then
		local daip = daip()
		local dase = dase() or ""
		local cn_port = cn_port()
		local core_type = uci:get("openclash", "config", "core_type") or "Dev"
		mode = luci.http.formvalue("rule_mode")
		if mode == script and core_type ~= "TUN" then luci.http.status(500, "Switch Faild") return end
		if not daip or not cn_port then luci.http.status(500, "Switch Faild") return end
		info = luci.sys.exec(string.format('curl -sL -m 3 -H "Content-Type: application/json" -H "Authorization: Bearer %s" -XPATCH http://"%s":"%s"/configs -d \'{\"mode\": \"%s\"}\'', dase, daip, cn_port, mode))
		if info ~= "" then
			luci.http.status(500, "Switch Faild")
		end
	else
		luci.http.status(500, "Switch Faild")
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		info = info;
	})
end

function action_get_run_mode()
	if mode() then
		luci.http.prepare_content("application/json")
		luci.http.write_json({
			clash = is_running(),
			watchdog = is_watchdog(),
			mode = mode();
		})
	else
		luci.http.status(500, "Get Faild")
		return
	end
end

function action_switch_run_mode()
	local mode, operation_mode
	if is_running() then
		mode = luci.http.formvalue("run_mode")
		operation_mode = uci:get("openclash", "config", "operation_mode")
		if operation_mode == "redir-host" then
			uci:set("openclash", "config", "en_mode", "redir-host"..mode)
		elseif operation_mode == "fake-ip" then
			uci:set("openclash", "config", "en_mode", "fake-ip"..mode)
		end
		uci:commit("openclash")
		luci.sys.exec("/etc/init.d/openclash restart >/dev/null 2>&1 &")
	else
		luci.http.status(500, "Switch Faild")
		return
	end
end

function action_log_level()
	local level, info
	if is_running() then
		local daip = daip()
		local dase = dase() or ""
		local cn_port = cn_port()
		if not daip or not cn_port then return end
		info = json.parse(luci.sys.exec(string.format('curl -sL -m 3 -H "Content-Type: application/json" -H "Authorization: Bearer %s" -XGET http://"%s":"%s"/configs', dase, daip, cn_port)))
		if info then
			level = info["log-level"]
		else
			level = uci:get("openclash", "config", "log_level") or "info"
		end
	else
		level = uci:get("openclash", "config", "log_level") or "info"
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		log_level = level;
	})
end

function action_switch_log()
	local level, info
	if is_running() then
		local daip = daip()
		local dase = dase() or ""
		local cn_port = cn_port()
		level = luci.http.formvalue("log_level")
		if not daip or not cn_port then luci.http.status(500, "Switch Faild") return end
		info = luci.sys.exec(string.format('curl -sL -m 3 -H "Content-Type: application/json" -H "Authorization: Bearer %s" -XPATCH http://"%s":"%s"/configs -d \'{\"log-level\": \"%s\"}\'', dase, daip, cn_port, level))
		if info ~= "" then
			luci.http.status(500, "Switch Faild")
		end
	else
		luci.http.status(500, "Switch Faild")
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		info = info;
	})
end

local function s(e)
local t=0
local a={' B/S',' KB/S',' MB/S',' GB/S',' TB/S'}
if (e<=1024) then
	return e..a[1]
else
repeat
e=e/1024
t=t+1
until(e<=1024)
return string.format("%.1f",e)..a[t]
end
end

function action_toolbar_show_sys()
	local pid = luci.sys.exec("pidof clash |head -1 |tr -d '\n' 2>/dev/null")
	local mem, cpu
	if pid and pid ~= "" then
		mem = tonumber(luci.sys.exec(string.format("cat /proc/%s/status 2>/dev/null |grep -w VmRSS |awk '{print $2}'", pid)))
		cpu = luci.sys.exec(string.format("top -b -n1 |grep -E '%s' 2>/dev/null |grep -v grep |awk '{for (i=1;i<=NF;i++) {if ($i ~ /clash/) break; else cpu=i}}; {print $cpu}' 2>/dev/null", pid))
		if mem and cpu then
			mem = fs.filesize(mem*1024) or "0 KB"
			cpu = string.match(cpu, "%d+") or "0"
		else
			mem = "0 KB"
			cpu = "0"
		end
	else
		return
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		mem = mem,
		cpu = cpu;
	})
end

function action_toolbar_show()
	local pid = luci.sys.exec("pidof clash |head -1 |tr -d '\n' 2>/dev/null")
	local traffic, connections, connection, up, down, up_total, down_total, mem, cpu
	if pid and pid ~= "" then
		local daip = daip()
		local dase = dase() or ""
		local cn_port = cn_port()
		if not daip or not cn_port then return end
		traffic = json.parse(luci.sys.exec(string.format('curl -sL -m 3 -H "Content-Type: application/json" -H "Authorization: Bearer %s" -XGET http://"%s":"%s"/traffic', dase, daip, cn_port)))
		connections = json.parse(luci.sys.exec(string.format('curl -sL -m 3 -H "Content-Type: application/json" -H "Authorization: Bearer %s" -XGET http://"%s":"%s"/connections', dase, daip, cn_port)))
		if traffic and connections then
			connection = #(connections.connections)
			up = s(traffic.up)
			down = s(traffic.down)
			up_total = fs.filesize(connections.uploadTotal)
			down_total = fs.filesize(connections.downloadTotal)
		else
			up = "0 B/S"
			down = "0 B/S"
			up_total = "0 KB"
			down_total = "0 KB"
			connection = "0"
		end
		mem = tonumber(luci.sys.exec(string.format("cat /proc/%s/status 2>/dev/null |grep -w VmRSS |awk '{print $2}'", pid)))
		cpu = luci.sys.exec(string.format("top -b -n1 |grep -E '%s' 2>/dev/null |grep -v grep |awk '{for (i=1;i<=NF;i++) {if ($i ~ /clash/) break; else cpu=i}}; {print $cpu}' 2>/dev/null", pid))
		if mem and cpu then
			mem = fs.filesize(mem*1024) or "0 KB"
			cpu = string.match(cpu, "%d+") or  "0"
		else
			mem = "0 KB"
			cpu = "0"
		end
	else
		return
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		connections = connection,
		up = up,
		down = down,
		up_total = up_total,
		down_total = down_total,
		mem = mem,
		cpu = cpu;
	})
end

function action_config_name()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		config_name = config_name(),
		config_path = config_path();
	})
end

function action_save_corever_branch()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		save_corever_branch = save_corever_branch();
	})
end

function action_dler_login_info_save()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		dler_login_info_save = dler_login_info_save();
	})
end

function action_dler_info()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		dler_info = dler_info();
	})
end

function action_dler_checkin()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		dler_checkin = dler_checkin();
	})
end

function action_dler_logout()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		dler_logout = dler_logout();
	})
end

function action_dler_login()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		dler_login = dler_login();
	})
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

function action_dashboard_type()
	local dashboard_type = uci:get("openclash", "config", "dashboard_type") or "Official"
	local yacd_type = uci:get("openclash", "config", "yacd_type") or "Official"
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		dashboard_type = dashboard_type,
		yacd_type = yacd_type;
	})
end

function action_switch_dashboard()
	local switch_name = luci.http.formvalue("name")
	local switch_type = luci.http.formvalue("type")
	local state = luci.sys.call(string.format('/usr/share/openclash/openclash_download_dashboard.sh "%s" "%s" >/dev/null 2>&1', switch_name, switch_type))
	if switch_name == "Dashboard" and tonumber(state) == 1 then
		if switch_type == "Official" then
			uci:set("openclash", "config", "dashboard_type", "Official")
			uci:commit("openclash")
		else
			uci:set("openclash", "config", "dashboard_type", "Meta")
			uci:commit("openclash")
		end
	elseif tonumber(state) == 1 then
		if switch_type == "Official" then
			uci:set("openclash", "config", "yacd_type", "Official")
			uci:commit("openclash")
		else
			uci:set("openclash", "config", "yacd_type", "Meta")
			uci:commit("openclash")
		end
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		download_state = state;
	})
end

function action_op_mode()
	local op_mode = uci:get("openclash", "config", "operation_mode")
	luci.http.prepare_content("application/json")
	luci.http.write_json({
	  op_mode = op_mode;
	})
end

function action_switch_mode()
	local switch_mode = uci:get("openclash", "config", "operation_mode")
	if switch_mode == "redir-host" then
		uci:set("openclash", "config", "operation_mode", "fake-ip")
		uci:commit("openclash")
	else
		uci:set("openclash", "config", "operation_mode", "redir-host")
		uci:commit("openclash")
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
		db_foward_port = db_foward_port(),
		db_foward_domain = db_foward_domain(),
		db_forward_ssl = db_foward_ssl(),
		web = is_web(),
		cn_port = cn_port(),
		restricted_mode = restricted_mode();
	})
end

function action_state()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		lhie1 = lhie1(),
		ConnersHua = ConnersHua(),
		ConnersHua_return = ConnersHua_return(),
		ipdb = ipdb(),
		geosite = geosite(),
		historychecktime = historychecktime(),
		chnroutev6 = chnroutev6(),
		chnroute = chnroute();
	})
end

function action_lastversion()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
			lastversion = check_lastversion();
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
			corecv = corecv(),
			coretuncv = coretuncv(),
			coremetacv = coremetacv(),
			coremodel = coremodel(),
			opcv = opcv(),
			upchecktime = upchecktime(),
			corelv = corelv(),
			oplv = oplv();
	})
end

function action_update_info()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
			corever = corever(),
			release_branch = release_branch();
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

function action_close_all_connection()
	return luci.sys.call("sh /usr/share/openclash/openclash_history_get.sh 'close_all_conection'")
end

function action_reload_firewall()
	return luci.sys.call("/etc/init.d/openclash reload 'firewall'")
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

function action_update_geosite()
	return luci.sys.call("/usr/share/openclash/openclash_geosite.sh >/dev/null 2>&1")
end

function action_download_rule()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		rule_download_status = download_rule();
	})
end

function action_download_netflix_domains()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		rule_download_status = download_netflix_domains();
	})
end

function action_download_disney_domains()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		rule_download_status = download_disney_domains();
	})
end

function action_refresh_log()
	luci.http.prepare_content("application/json")
	local logfile="/tmp/openclash.log"
	local file = io.open(logfile, "r+")
	local info, len, line, lens, cache, ex_match, line_trans
	local data = ""
	local limit = 1000
	local log_tb = {}
	local log_len = tonumber(luci.http.formvalue("log_len")) or 0
	if file == nil then
 		return nil
 	end
 	file:seek("set")
 	info = file:read("*all")
 	info = info:reverse()
 	file:close()
 	cache, len = string.gsub(info, '[^\n]+', "")
 	if len == log_len then return nil end
	if log_len == 0 then
		if len > limit then lens = limit else lens = len end
	else
		lens = len - log_len
	end
	string.gsub(info, '[^\n]+', function(w) table.insert(log_tb, w) end, lens)
	for i=1, lens do
		line = log_tb[i]:reverse()
		line_trans = line
		ex_match = false
		core_match = false
		time_format = false
		while true do
			ex_keys = {"UDP%-Receive%-Buffer%-Size", "^Sec%-Fetch%-Mode", "^User%-Agent", "^Access%-Control", "^Accept", "^Origin", "^Referer", "^Connection", "^Pragma", "^Cache-"}
			for key=1, #ex_keys do
				if string.find (line, ex_keys[key]) then
					ex_match = true
					break
				end
			end
    		if ex_match then break end

			core_keys = {" DBG ", " INF ", "level=", " WRN ", " ERR ", " FTL "}
			for key=1, #core_keys do
				if string.find(string.sub(line, 0, 13), core_keys[key]) or (string.find(line, core_keys[key]) and core_keys[key] == "level=") then
					core_match = true
					if core_keys[key] ~= "level=" then
						time_format = true
					end
					break
				end
			end
			if time_format then
				if string.match(string.sub(line, 0, 8), "%d%d:%d%d:%d%d") then
					line_trans = '"'..os.date("%Y-%m-%d %H:%M:%S", tonumber(string.sub(line, 0, 8)))..'"'..string.sub(line, 9, -1)
				end
			end
			if not core_match then
				if not string.find (line, "【") or not string.find (line, "】") then
					line_trans = trans_line_nolabel(line)
				else
					line_trans = trans_line(line)
				end
			end
			if data == "" then
				data = line_trans
			elseif log_len == 0 and i == limit then
				data = data .."\n" .. line_trans .. "\n..."
			else
				data = data .."\n" .. line_trans
			end
    		break
    	end
	end
	luci.http.write_json({
		len = len,
		log = data;
	})
end

function action_del_log()
	luci.sys.exec(": > /tmp/openclash.log")
	return
end

function action_del_start_log()
	luci.sys.exec(": > /tmp/openclash_start.log")
	return
end

function split(str,delimiter)
	local dLen = string.len(delimiter)
	local newDeli = ''
	for i=1,dLen,1 do
		newDeli = newDeli .. "["..string.sub(delimiter,i,i).."]"
	end

	local locaStart,locaEnd = string.find(str,newDeli)
	local arr = {}
	local n = 1
	while locaStart ~= nil
	do
		if locaStart>0 then
			arr[n] = string.sub(str,1,locaStart-1)
			n = n + 1
		end

		str = string.sub(str,locaEnd+1,string.len(str))
		locaStart,locaEnd = string.find(str,newDeli)
	end
	if str ~= nil then
		arr[n] = str
	end
	return arr
end

function action_write_netflix_domains()
	local domains = luci.http.formvalue("domains")
	local dustom_file = "/etc/openclash/custom/openclash_custom_netflix_domains.list"
	local file = io.open(dustom_file, "a+")
	file:seek("set")
	local domain = file:read("*a")
	for v, k in pairs(split(domains,"\n")) do
		if not string.find(domain,k,1,true) then
			file:write(k.."\n")
		end
	end
	file:close()
	return
end

function action_catch_netflix_domains()
	local cmd = "/usr/share/openclash/openclash_debug_getcon.lua 'netflix-nflxvideo'"
	luci.http.prepare_content("text/plain")
	local util = io.popen(cmd)
	if util and util ~= "" then
		while true do
			local ln = util:read("*l")
			if not ln then break end
			luci.http.write(ln)
			luci.http.write(",")
		end
		util:close()
		return
	end
	luci.http.status(500, "Bad address")
end

function action_diag_connection()
	local addr = luci.http.formvalue("addr")
	if addr and (datatype.hostname(addr) or datatype.ipaddr(addr)) then
		local cmd = string.format("/usr/share/openclash/openclash_debug_getcon.lua %s", addr)
		luci.http.prepare_content("text/plain")
		local util = io.popen(cmd)
		if util and util ~= "" then
			while true do
				local ln = util:read("*l")
				if not ln then break end
				luci.http.write(ln)
				luci.http.write("\n")
			end
			util:close()
		end
		return
	end
	luci.http.status(500, "Bad address")
end

function action_diag_dns()
	local addr = luci.http.formvalue("addr")
	if addr and datatype.hostname(addr)then
		local cmd = string.format("/usr/share/openclash/openclash_debug_dns.lua %s", addr)
		luci.http.prepare_content("text/plain")
		local util = io.popen(cmd)
		if util and util ~= "" then
			while true do
				local ln = util:read("*l")
				if not ln then break end
				luci.http.write(ln)
				luci.http.write("\n")
			end
			util:close()
		end
		return
	end
	luci.http.status(500, "Bad address")
end

function action_gen_debug_logs()
	local gen_log = luci.sys.call("/usr/share/openclash/openclash_debug.sh")
	if not gen_log then return end
	local logfile = "/tmp/openclash_debug.log"
	if not fs.access(logfile) then
		return
	end
	luci.http.prepare_content("text/plain; charset=utf-8")
	local file=io.open(logfile, "r+")
	file:seek("set")
	local info = ""
	for line in file:lines() do
		if info ~= "" then
			info = info.."\n"..line
		else
			info = line
		end
	end
	file:close()
	luci.http.write(info)
end

function action_backup()
	local config = luci.sys.call("cp /etc/config/openclash /etc/openclash/openclash >/dev/null 2>&1")
	local reader = ltn12_popen("tar -C '/etc/openclash/' -cz . 2>/dev/null")

	luci.http.header(
		'Content-Disposition', 'attachment; filename="Backup-OpenClash-%s.tar.gz"' %{
			os.date("%Y-%m-%d-%H-%M-%S")
		})

	luci.http.prepare_content("application/x-targz")
	luci.ltn12.pump.all(reader, luci.http.write)
	luci.sys.call("rm -rf /etc/openclash/openclash >/dev/null 2>&1")
end

function action_backup_ex_core()
	local config = luci.sys.call("cp /etc/config/openclash /etc/openclash/openclash >/dev/null 2>&1")
	local reader = ltn12_popen("echo 'core' > /tmp/oc_exclude.txt && tar -C '/etc/openclash/' -X '/tmp/oc_exclude.txt' -cz . 2>/dev/null")

	luci.http.header(
		'Content-Disposition', 'attachment; filename="Backup-OpenClash-Exclude-Cores-%s.tar.gz"' %{
			os.date("%Y-%m-%d-%H-%M-%S")
		})

	luci.http.prepare_content("application/x-targz")
	luci.ltn12.pump.all(reader, luci.http.write)
	luci.sys.call("rm -rf /etc/openclash/openclash >/dev/null 2>&1")
end

function action_backup_only_config()
	local reader = ltn12_popen("tar -C '/etc/openclash' -cz './config' 2>/dev/null")

	luci.http.header(
		'Content-Disposition', 'attachment; filename="Backup-OpenClash-Config-%s.tar.gz"' %{
			os.date("%Y-%m-%d-%H-%M-%S")
		})

	luci.http.prepare_content("application/x-targz")
	luci.ltn12.pump.all(reader, luci.http.write)
end

function action_backup_only_core()
	local reader = ltn12_popen("tar -C '/etc/openclash' -cz './core' 2>/dev/null")

	luci.http.header(
		'Content-Disposition', 'attachment; filename="Backup-OpenClash-Cores-%s.tar.gz"' %{
			os.date("%Y-%m-%d-%H-%M-%S")
		})

	luci.http.prepare_content("application/x-targz")
	luci.ltn12.pump.all(reader, luci.http.write)
end

function action_backup_only_rule()
	local reader = ltn12_popen("tar -C '/etc/openclash' -cz './rule_provider' 2>/dev/null")

	luci.http.header(
		'Content-Disposition', 'attachment; filename="Backup-OpenClash-Only-Rule-Provider-%s.tar.gz"' %{
			os.date("%Y-%m-%d-%H-%M-%S")
		})

	luci.http.prepare_content("application/x-targz")
	luci.ltn12.pump.all(reader, luci.http.write)
end

function action_backup_only_proxy()
	local reader = ltn12_popen("tar -C '/etc/openclash' -cz './proxy_provider' 2>/dev/null")

	luci.http.header(
		'Content-Disposition', 'attachment; filename="Backup-OpenClash-Proxy-Provider-%s.tar.gz"' %{
			os.date("%Y-%m-%d-%H-%M-%S")
		})

	luci.http.prepare_content("application/x-targz")
	luci.ltn12.pump.all(reader, luci.http.write)
end

function ltn12_popen(command)

	local fdi, fdo = nixio.pipe()
	local pid = nixio.fork()

	if pid > 0 then
		fdo:close()
		local close
		return function()
			local buffer = fdi:read(2048)
			local wpid, stat = nixio.waitpid(pid, "nohang")
			if not close and wpid and stat == "exited" then
				close = true
			end

			if buffer and #buffer > 0 then
				return buffer
			elseif close then
				fdi:close()
				return nil
			end
		end
	elseif pid == 0 then
		nixio.dup(fdo, nixio.stdout)
		fdi:close()
		fdo:close()
		nixio.exec("/bin/sh", "-c", command)
	end
end

function create_file()
	local file_name = luci.http.formvalue("filename")
	local file_path = luci.http.formvalue("filepath")..file_name
	fs.writefile(file_path, "")
	if not fs.isfile(file_path) then
		luci.http.status(500, "Create File Faild")
	end
	return
end

function rename_file()
	local new_file_name = luci.http.formvalue("new_file_name")
	local file_path = luci.http.formvalue("file_path")
	local old_file_name = luci.http.formvalue("file_name")
	local old_file_path = file_path .. old_file_name
	local new_file_path = file_path .. new_file_name
	local old_run_file_path = "/etc/openclash/" .. old_file_name
	local new_run_file_path = "/etc/openclash/" .. new_file_name
	local old_backup_file_path = "/etc/openclash/backup/" .. old_file_name
	local new_backup_file_path = "/etc/openclash/backup/" .. new_file_name
	if fs.rename(old_file_path, new_file_path) then
		if file_path == "/etc/openclash/config/" then
			if uci:get("openclash", "config", "config_path") == old_file_path then
				uci:set("openclash", "config", "config_path", new_file_path)
			end
			
			if fs.isfile(old_run_file_path) then
				fs.rename(old_run_file_path, new_run_file_path)
			end
			
			if fs.isfile(old_backup_file_path) then
				fs.rename(old_backup_file_path, new_backup_file_path)
			end
			
			uci:foreach("openclash", "config_subscribe",
			function(s)
				if s.name == fs.filename(old_file_name) and fs.filename(new_file_name) ~= new_file_name then
					uci:set("openclash", s[".name"], "name", fs.filename(new_file_name))
				end
			end)
			
			uci:foreach("openclash", "other_rules",
			function(s)
				if s.config == old_file_name and fs.filename(new_file_name) ~= new_file_name then
					uci:set("openclash", s[".name"], "config", new_file_name)
				end
			end)
			
			uci:foreach("openclash", "groups",
			function(s)
				if s.config == old_file_name and fs.filename(new_file_name) ~= new_file_name then
					uci:set("openclash", s[".name"], "config", new_file_name)
				end
			end)
			
			uci:foreach("openclash", "proxy-provider",
			function(s)
				if s.config == old_file_name and fs.filename(new_file_name) ~= new_file_name then
					uci:set("openclash", s[".name"], "config", new_file_name)
				end
			end)
			
			uci:foreach("openclash", "rule_provider_config",
			function(s)
				if s.config == old_file_name and fs.filename(new_file_name) ~= new_file_name then
					uci:set("openclash", s[".name"], "config", new_file_name)
				end
			end)
			
			uci:foreach("openclash", "servers",
			function(s)
				if s.config == old_file_name and fs.filename(new_file_name) ~= new_file_name then
					uci:set("openclash", s[".name"], "config", new_file_name)
				end
			end)
			
			uci:foreach("openclash", "game_config",
			function(s)
				if s.config == old_file_name and fs.filename(new_file_name) ~= new_file_name then
					uci:set("openclash", s[".name"], "config", new_file_name)
				end
			end)
			
			uci:foreach("openclash", "rule_providers",
			function(s)
				if s.config == old_file_name and fs.filename(new_file_name) ~= new_file_name then
					uci:set("openclash", s[".name"], "config", new_file_name)
				end
			end)
			
			uci:commit("openclash")
		end
		luci.http.status(200, "Rename File Successful")
	else
		luci.http.status(500, "Rename File Faild")
	end
	return
end

function manual_stream_unlock_test()
	local type = luci.http.formvalue("type")
	local cmd = string.format('/usr/share/openclash/openclash_streaming_unlock.lua "%s"', type)
	local line_trans
	luci.http.prepare_content("text/plain; charset=utf-8")
	local util = io.popen(cmd)
	if util and util ~= "" then
		while true do
			local ln = util:read("*l")
			if ln then
				if not string.find (ln, "【") or not string.find (ln, "】") then
					line_trans = trans_line_nolabel(ln)
   				else
   					line_trans = trans_line(ln)
   				end
				luci.http.write(line_trans)
				luci.http.write("\n")
			end
			if not process_status("openclash_streaming_unlock.lua "..type) or not process_status("openclash_streaming_unlock.lua ") then
				break
			end
		end
		util:close()
		return
	end
	luci.http.status(500, "Something Wrong While Testing...")
end

function all_proxies_stream_test()
	local type = luci.http.formvalue("type")
	local cmd = string.format('/usr/share/openclash/openclash_streaming_unlock.lua "%s" "%s"', type, "all")
	local line_trans
	luci.http.prepare_content("text/plain; charset=utf-8")
	local util = io.popen(cmd)
	if util and util ~= "" then
		while true do
			local ln = util:read("*l")
			if ln then
				if not string.find (ln, "【") or not string.find (ln, "】") then
					line_trans = trans_line_nolabel(ln)
   				else
   					line_trans = trans_line(ln)
   				end
				luci.http.write(line_trans)
				luci.http.write("\n")
			end
			if not process_status("openclash_streaming_unlock.lua "..type) or not process_status("openclash_streaming_unlock.lua ") then
				break
			end
		end
		util:close()
		return
	end
	luci.http.status(500, "Something Wrong While Testing...")
end

function trans_line_nolabel(data)
	local line_trans = ""
	if string.match(string.sub(data, 0, 19), "%d%d%d%d%-%d%d%-%d%d %d%d:%d%d:%d%d") then
		line_trans = string.sub(data, 0, 20)..luci.i18n.translate(string.sub(data, 21, -1))
	else
		line_trans = luci.i18n.translate(string.sub(data, 0, -1))
	end
	return line_trans
end

function trans_line(data)
	local no_trans = {}
	local line_trans = ""
	local a = string.find (data, "【")
	local b = string.find (data, "】") + 2
	local c = 21
	local d = 0
	local v
	local x
	while true do
		table.insert(no_trans, a)
		table.insert(no_trans, b)
		if string.find (data, "【", b+1) and string.find (data, "】", b+1) then
			a = string.find (data, "【", b+1)
			b = string.find (data, "】", b+1) + 2
		else
			break
		end
	end
	for k = 1, #no_trans, 2 do
		x = no_trans[k]
		v = no_trans[k+1]
		if x <= 21 or not string.match(string.sub(data, 0, 19), "%d%d%d%d%-%d%d%-%d%d %d%d:%d%d:%d%d") then
			line_trans = line_trans .. luci.i18n.translate(string.sub(data, d, x - 1)) .. string.sub(data, x, v)
			d = v + 1
		elseif v <= string.len(data) then
			line_trans = line_trans .. luci.i18n.translate(string.sub(data, c, x - 1)) .. string.sub(data, x, v)
		end
		c = v + 1
	end
	if c > string.len(data) then
		if d == 0 then
			if string.match(string.sub(data, 0, 19), "%d%d%d%d%-%d%d%-%d%d %d%d:%d%d:%d%d") then
				line_trans = string.sub(data, 0, 20) .. line_trans
			end
		end
	else
		if d == 0 then
			if string.match(string.sub(data, 0, 19), "%d%d%d%d%-%d%d%-%d%d %d%d:%d%d:%d%d") then
				line_trans = string.sub(data, 0, 20) .. line_trans .. luci.i18n.translate(string.sub(data, c, -1))
			end
		else
			line_trans = line_trans .. luci.i18n.translate(string.sub(data, c, -1))
		end
	end
	return line_trans
end

function process_status(name)
	local ps_version = luci.sys.exec("ps --version 2>&1 |grep -c procps-ng |tr -d '\n'")
	if ps_version == "1" then
		return luci.sys.call(string.format("ps -efw |grep '%s' |grep -v grep >/dev/null", name)) == 0
	else
		return luci.sys.call(string.format("ps -w |grep '%s' |grep -v grep >/dev/null", name)) == 0
	end
end
