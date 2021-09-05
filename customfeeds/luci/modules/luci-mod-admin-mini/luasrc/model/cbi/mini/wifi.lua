-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

-- Data init --

local fs  = require "nixio.fs"
local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()

if not uci:get("network", "wan") then
	uci:section("network", "interface", "wan", {proto="none", ifname=" "})
	uci:save("network")
	uci:commit("network")
end

local wlcursor = luci.model.uci.cursor_state()
local wireless = wlcursor:get_all("wireless")
local wifidevs = {}
local ifaces = {}

for k, v in pairs(wireless) do
	if v[".type"] == "wifi-iface" then
		table.insert(ifaces, v)
	end
end

wlcursor:foreach("wireless", "wifi-device",
	function(section)
		table.insert(wifidevs, section[".name"])
	end)


-- Main Map --

m = Map("wireless", translate("Wireless"), translate("Here you can configure installed wifi devices."))
m:chain("network")


-- Status Table --
s = m:section(Table, ifaces, translate("Networks"))

link = s:option(DummyValue, "_link", translate("Link"))
function link.cfgvalue(self, section)
	local ifname = self.map:get(section, "ifname")
	local iwinfo = sys.wifi.getiwinfo(ifname)
	return iwinfo and "%d/%d" %{ iwinfo.quality, iwinfo.quality_max } or "-"
end

essid = s:option(DummyValue, "ssid", "ESSID")

bssid = s:option(DummyValue, "_bsiid", "BSSID")
function bssid.cfgvalue(self, section)
	local ifname = self.map:get(section, "ifname")
	local iwinfo = sys.wifi.getiwinfo(ifname)
	return iwinfo and iwinfo.bssid or "-"
end

channel = s:option(DummyValue, "channel", translate("Channel"))
function channel.cfgvalue(self, section)
	return wireless[self.map:get(section, "device")].channel
end

protocol = s:option(DummyValue, "_mode", translate("Protocol"))
function protocol.cfgvalue(self, section)
	local mode = wireless[self.map:get(section, "device")].mode
	return mode and "802." .. mode
end

mode = s:option(DummyValue, "mode", translate("Mode"))
encryption = s:option(DummyValue, "encryption", translate("<abbr title=\"Encrypted\">Encr.</abbr>"))

power = s:option(DummyValue, "_power", translate("Power"))
function power.cfgvalue(self, section)
	local ifname = self.map:get(section, "ifname")
	local iwinfo = sys.wifi.getiwinfo(ifname)
	return iwinfo and "%d dBm" % iwinfo.txpower or "-"
end

scan = s:option(Button, "_scan", translate("Scan"))
scan.inputstyle = "find"

function scan.cfgvalue(self, section)
	return self.map:get(section, "ifname") or false
end

-- WLAN-Scan-Table --

t2 = m:section(Table, {}, translate("<abbr title=\"Wireless Local Area Network\">WLAN</abbr>-Scan"), translate("Wifi networks in your local environment"))

function scan.write(self, section)
	m.autoapply = false
	t2.render = t2._render
	local ifname = self.map:get(section, "ifname")
	local iwinfo = sys.wifi.getiwinfo(ifname)
	if iwinfo then
		local _, cell
		for _, cell in ipairs(iwinfo.scanlist) do
			t2.data[#t2.data+1] = {
				Quality = "%d/%d" %{ cell.quality, cell.quality_max },
				ESSID   = cell.ssid,
				Address = cell.bssid,
				Mode    = cell.mode,
				["Encryption key"] = cell.encryption.enabled and "On" or "Off",
				["Signal level"]   = "%d dBm" % cell.signal,
				["Noise level"]    = "%d dBm" % iwinfo.noise
			}
		end
	end
end

t2._render = t2.render
t2.render = function() end

t2:option(DummyValue, "Quality", translate("Link"))
essid = t2:option(DummyValue, "ESSID", "ESSID")
function essid.cfgvalue(self, section)
	return self.map:get(section, "ESSID")
end

t2:option(DummyValue, "Address", "BSSID")
t2:option(DummyValue, "Mode", translate("Mode"))
chan = t2:option(DummyValue, "channel", translate("Channel"))
function chan.cfgvalue(self, section)
	return self.map:get(section, "Channel")
	    or self.map:get(section, "Frequency")
	    or "-"
end

t2:option(DummyValue, "Encryption key", translate("<abbr title=\"Encrypted\">Encr.</abbr>"))

t2:option(DummyValue, "Signal level", translate("Signal"))

t2:option(DummyValue, "Noise level", translate("Noise"))



if #wifidevs < 1 then
	return m
end

-- Config Section --

s = m:section(NamedSection, wifidevs[1], "wifi-device", translate("Devices"))
s.addremove = false

en = s:option(Flag, "disabled", translate("enable"))
en.rmempty = false
en.enabled = "0"
en.disabled = "1"

function en.cfgvalue(self, section)
	return Flag.cfgvalue(self, section) or "0"
end


local hwtype = m:get(wifidevs[1], "type")

ch = s:option(Value, "channel", translate("Channel"))
for i=1, 14 do
	ch:value(i, i .. " (2.4 GHz)")
end


s = m:section(TypedSection, "wifi-iface", translate("Local Network"))
s.anonymous = true
s.addremove = false

s:option(Value, "ssid", translate("Network Name (<abbr title=\"Extended Service Set Identifier\">ESSID</abbr>)"))

bssid = s:option(Value, "bssid", translate("<abbr title=\"Basic Service Set Identifier\">BSSID</abbr>"))

local devs = {}
luci.model.uci.cursor():foreach("wireless", "wifi-device",
	function (section)
		table.insert(devs, section[".name"])
	end)

if #devs > 1 then
	device = s:option(DummyValue, "device", translate("Device"))
else
	s.defaults.device = devs[1]
end

mode = s:option(ListValue, "mode", translate("Mode"))
mode.override_values = true
mode:value("ap", translate("Provide (Access Point)"))
mode:value("adhoc", translate("Independent (Ad-Hoc)"))
mode:value("sta", translate("Join (Client)"))

function mode.write(self, section, value)
	if value == "sta" then
		local oldif = m.uci:get("network", "wan", "ifname")
		if oldif and oldif ~= " " then
			m.uci:set("network", "wan", "_ifname", oldif)
		end
		m.uci:set("network", "wan", "ifname", " ")

		self.map:set(section, "network", "wan")
	else
		if m.uci:get("network", "wan", "_ifname") then
			m.uci:set("network", "wan", "ifname", m.uci:get("network", "wan", "_ifname"))
		end
		self.map:set(section, "network", "lan")
	end

	return ListValue.write(self, section, value)
end

encr = s:option(ListValue, "encryption", translate("Encryption"))
encr.override_values = true
encr:value("none", "No Encryption")
encr:value("wep", "WEP")

if hwtype == "mac80211" then
	local supplicant = fs.access("/usr/sbin/wpa_supplicant")
	local hostapd    = fs.access("/usr/sbin/hostapd")

	if hostapd and supplicant then
		encr:value("psk", "WPA-PSK")
		encr:value("psk2", "WPA2-PSK")
		encr:value("psk-mixed", "WPA-PSK/WPA2-PSK Mixed Mode")
		encr:value("wpa", "WPA-Radius", {mode="ap"}, {mode="sta"})
		encr:value("wpa2", "WPA2-Radius", {mode="ap"}, {mode="sta"})
	elseif hostapd and not supplicant then
		encr:value("psk", "WPA-PSK", {mode="ap"}, {mode="adhoc"})
		encr:value("psk2", "WPA2-PSK", {mode="ap"}, {mode="adhoc"})
		encr:value("psk-mixed", "WPA-PSK/WPA2-PSK Mixed Mode", {mode="ap"}, {mode="adhoc"})
		encr:value("wpa", "WPA-Radius", {mode="ap"})
		encr:value("wpa2", "WPA2-Radius", {mode="ap"})
		encr.description = translate(
			"WPA-Encryption requires wpa_supplicant (for client mode) or hostapd (for AP " ..
			"and ad-hoc mode) to be installed."
		)
	elseif not hostapd and supplicant then
		encr:value("psk", "WPA-PSK", {mode="sta"})
		encr:value("psk2", "WPA2-PSK", {mode="sta"})
		encr:value("psk-mixed", "WPA-PSK/WPA2-PSK Mixed Mode", {mode="sta"})
		encr:value("wpa", "WPA-EAP", {mode="sta"})
		encr:value("wpa2", "WPA2-EAP", {mode="sta"})
		encr.description = translate(
			"WPA-Encryption requires wpa_supplicant (for client mode) or hostapd (for AP " ..
			"and ad-hoc mode) to be installed."
		)		
	else
		encr.description = translate(
			"WPA-Encryption requires wpa_supplicant (for client mode) or hostapd (for AP " ..
			"and ad-hoc mode) to be installed."
		)
	end
elseif hwtype == "broadcom" then
	encr:value("psk", "WPA-PSK")
	encr:value("psk2", "WPA2-PSK")
	encr:value("psk+psk2", "WPA-PSK/WPA2-PSK Mixed Mode")
elseif hwtype == "mt_dbdc" then
	encr:value("psk", "WPA-PSK")
	encr:value("psk2", "WPA2-PSK")
	encr:value("psk-mixed", "WPA-PSK/WPA2-PSK Mixed Mode")
end

key = s:option(Value, "key", translate("Key"))
key:depends("encryption", "wep")
key:depends("encryption", "psk")
key:depends("encryption", "psk2")
key:depends("encryption", "psk+psk2")
key:depends("encryption", "psk-mixed")
key:depends({mode="ap", encryption="wpa"})
key:depends({mode="ap", encryption="wpa2"})
key.rmempty = true
key.password = true

server = s:option(Value, "server", translate("Radius-Server"))
server:depends({mode="ap", encryption="wpa"})
server:depends({mode="ap", encryption="wpa2"})
server.rmempty = true

port = s:option(Value, "port", translate("Radius-Port"))
port:depends({mode="ap", encryption="wpa"})
port:depends({mode="ap", encryption="wpa2"})
port.rmempty = true


if hwtype == "mac80211" then
	nasid = s:option(Value, "nasid", translate("NAS ID"))
	nasid:depends({mode="ap", encryption="wpa"})
	nasid:depends({mode="ap", encryption="wpa2"})
	nasid.rmempty = true

	eaptype = s:option(ListValue, "eap_type", translate("EAP-Method"))
	eaptype:value("TLS")
	eaptype:value("TTLS")
	eaptype:value("PEAP")
	eaptype:depends({mode="sta", encryption="wpa"})
	eaptype:depends({mode="sta", encryption="wpa2"})

	cacert = s:option(FileUpload, "ca_cert", translate("Path to CA-Certificate"))
	cacert:depends({mode="sta", encryption="wpa"})
	cacert:depends({mode="sta", encryption="wpa2"})

	privkey = s:option(FileUpload, "priv_key", translate("Path to Private Key"))
	privkey:depends({mode="sta", eap_type="TLS", encryption="wpa2"})
	privkey:depends({mode="sta", eap_type="TLS", encryption="wpa"})

	privkeypwd = s:option(Value, "priv_key_pwd", translate("Password of Private Key"))
	privkeypwd:depends({mode="sta", eap_type="TLS", encryption="wpa2"})
	privkeypwd:depends({mode="sta", eap_type="TLS", encryption="wpa"})


	auth = s:option(Value, "auth", translate("Authentication"))
	auth:value("PAP")
	auth:value("CHAP")
	auth:value("MSCHAP")
	auth:value("MSCHAPV2")
	auth:depends({mode="sta", eap_type="PEAP", encryption="wpa2"})
	auth:depends({mode="sta", eap_type="PEAP", encryption="wpa"})
	auth:depends({mode="sta", eap_type="TTLS", encryption="wpa2"})
	auth:depends({mode="sta", eap_type="TTLS", encryption="wpa"})


	identity = s:option(Value, "identity", translate("Identity"))
	identity:depends({mode="sta", eap_type="PEAP", encryption="wpa2"})
	identity:depends({mode="sta", eap_type="PEAP", encryption="wpa"})
	identity:depends({mode="sta", eap_type="TTLS", encryption="wpa2"})
	identity:depends({mode="sta", eap_type="TTLS", encryption="wpa"})

	password = s:option(Value, "password", translate("Password"))
	password:depends({mode="sta", eap_type="PEAP", encryption="wpa2"})
	password:depends({mode="sta", eap_type="PEAP", encryption="wpa"})
	password:depends({mode="sta", eap_type="TTLS", encryption="wpa2"})
	password:depends({mode="sta", eap_type="TTLS", encryption="wpa"})
end


if hwtype == "broadcom" or hwtype == "mt_dbdc" then
	iso = s:option(Flag, "isolate", translate("AP-Isolation"), translate("Prevents Client to Client communication"))
	iso.rmempty = true
	iso:depends("mode", "ap")

	hide = s:option(Flag, "hidden", translate("Hide <abbr title=\"Extended Service Set Identifier\">ESSID</abbr>"))
	hide.rmempty = true
	hide:depends("mode", "ap")
end

if hwtype == "mac80211" then
	bssid:depends({mode="adhoc"})
end

if hwtype == "broadcom" then
	bssid:depends({mode="wds"})
	bssid:depends({mode="adhoc"})
end


return m
