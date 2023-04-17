
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()
local clash = "clash"
local http = luci.http

font_red = [[<font color="red">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]


k = Map(clash)
k.reset = false
k.submit = false
s=k:section(TypedSection, "clash", translate("Local Update GeoIP"))
s.anonymous = true
s.addremove=false
o = s:option(FileUpload, "")
o.description = translate("NB: Upload GEOIP Database file Country.mmdb")
.."<br />"
..translate("https://github.com/Dreamacro/maxmind-geoip/releases")
.."<br />"
..translate("https://raw.githubusercontent.com/alecthw/mmdb_china_ip_list/release/Country.mmdb")

o.title = translate("  ")
o.template = "clash/clash_upload"
um = s:option(DummyValue, "", nil)
um.template = "clash/clash_dvalue"

local dir, fd
dir = "/etc/clash/"
http.setfilehandler(
	function(meta, chunk, eof)
		if not fd then
			if not meta then return end

			if	meta and chunk then fd = nixio.open(dir .. meta.file, "w") end

			if not fd then
				um.value = translate("upload file error.")
				return
			end
		end
		if chunk and fd then
			fd:write(chunk)
		end
		if eof and fd then
			fd:close()
			fd = nil
			um.value = translate("File saved to") .. ' "/etc/clash/"'
			SYS.call("chmod + x /etc/clash/Country.mmdb")
			if luci.sys.call("pidof clash >/dev/null") == 0 then
			SYS.call("/etc/init.d/clash restart >/dev/null 2>&1 &")
			end
		end
	end
)

if luci.http.formvalue("upload") then
	local f = luci.http.formvalue("ulfile")
	if #f <= 0 then
		um.value = translate("No specify upload file.")
	end
end




m = Map("clash")
s = m:section(TypedSection, "clash" , translate("Online Update GeoIP"))
m.pageaction = false
s.anonymous = true
s.addremove=false

o = s:option(Flag, "auto_update_geoip", translate("Auto Update"))
o.description = translate("Auto Update GeoIP Database")

o = s:option(ListValue, "auto_update_geoip_time", translate("Update time (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=0
o.description = translate("GeoIP Update Time")


o = s:option(ListValue, "up_time", translate("Update Every (Week/Month)"))
o.rmempty = false
o.description = translate("Time For Update (Once a week/Month)")
o:value("weekly", translate("Weekly"))
o:value("monthly", translate("Monthly"))

o = s:option(ListValue, "geoip_update_day", translate("Update Day (Day of Week)"))
o:value("1", translate("Every Monday"))
o:value("2", translate("Every Tuesday"))
o:value("3", translate("Every Wednesday"))
o:value("4", translate("Every Thursday"))
o:value("5", translate("Every Friday"))
o:value("6", translate("Every Saturday"))
o:value("0", translate("Every Sunday"))
update_time = SYS.exec("ls -l --full-time /etc/clash/Country.mmdb|awk '{print $6,$7;}'")
o.description = translate("Update Time")..'- ' ..font_red..bold_on..update_time..bold_off..font_off..' '
o:depends("up_time", "weekly")

o = s:option(ListValue, "geo_update_week", translate("Update Day (Day of Month)"))
o:value("1", translate("Every 1st Day"))
o:value("7", translate("Every 7th Day"))
o:value("14", translate("Every 14th Day"))
o:value("21", translate("Every 21st Day"))
o:value("28", translate("Every 28th Day"))
update_time = SYS.exec("ls -l --full-time /etc/clash/Country.mmdb|awk '{print $6,$7;}'")
o.description = translate("Update Time")..'- ' ..font_red..bold_on..update_time..bold_off..font_off..' '
o:depends("up_time", "monthly")

o = s:option(ListValue, "geoip_source", translate("GeoIP Source"))
o:value("1", translate("MaxMind"))
o:value("2", translate("Github"))

o = s:option(Value, "license_key")
o.title = translate("License Key")
o.description = translate("MaxMind License Key")..translate(" https://www.maxmind.com/en/geolite2/signup")
o.rmempty = true
o:depends("geoip_source", "1")

o=s:option(Button,"update_geoip")
o.inputtitle = translate("Save & Apply")
o.title = luci.util.pcdata(translate("Save & Apply"))
o.inputstyle = "reload"
o.write = function()
  m.uci:commit("clash")
end

o = s:option(Button,"download")
o.title = translate("Download")
o.template = "clash/geoip"


return m, k

