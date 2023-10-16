local m, section, m2, s2

m = Map("modem", translate("Mobile Network"))
m.description = translate("Modem Server For OpenWrt")


-------------------------------Fibocom Wireless Inc. FM650 Module----------------------------------
if (string.gsub(luci.sys.exec('lsusb |grep "ID 2cb7:0a05 " | wc -l'),"%s+","")=="1") then
	section = m:section(TypedSection, "ndis", translate("FMFM650-CN Settings"), translate("[1]Automatic start upon startup: Check</br>[2] FMFMFM650-CN module default ECM (36) mode. If not, please modify it, otherwise dialing cannot proceed normally</br>"))
	section.anonymous = true
	section.addremove = false
	enable = section:option(Flag, "enabled", translate("Enable"))
	enable.rmempty  = false
else
	section = m:section(TypedSection, "ndis", translate("SIM Settings"), translate("Automatic operation upon startup \r\n ooo"))
	section.anonymous = true
	section.addremove = false
		section:tab("general", translate("General Setup"))
		section:tab("advanced", translate("Advanced Settings"))


	enable = section:taboption("general", Flag, "enabled", translate("Enable"))
	enable.rmempty  = false

	device = section:taboption("general",Value, "device", translate("Modem device"))
	device.rmempty = false
	local device_suggestions = nixio.fs.glob("/dev/cdc-wdm*")
	if device_suggestions then
		local node
		for node in device_suggestions do
			device:value(node)
		end
	end
	apn = section:taboption("general", Value, "apn", translate("APN"))
	username = section:taboption("general", Value, "username", translate("PAP/CHAP Username"))
	password = section:taboption("general", Value, "password", translate("PAP/CHAP Password"))
	password.password = true
	pincode = section:taboption("general", Value, "pincode", translate("PIN Code"))
	auth = section:taboption("general", Value, "auth", translate("Authentication Type"))
	auth.rmempty = true
	auth:value("", translate("-- Please choose --"))
	auth:value("both", "PAP/CHAP (both)")
	auth:value("pap", "PAP")
	auth:value("chap", "CHAP")
	auth:value("none", "NONE")
	tool = section:taboption("general", Value, "tool", translate("Tools"))
	tool:value("quectel-CM", "quectel-CM")
	tool.rmempty = true
	PdpType= section:taboption("general", Value, "pdptype", translate("PdpType"))
	PdpType:value("IPV4", "IPV4")
	PdpType:value("IPV6", "IPV6")
	PdpType:value("IPV4V6", "IPV4V6")
	PdpType.rmempty = true


	---------------------------advanced------------------------------
	bandlist = section:taboption("advanced", ListValue, "bandlist", translate("Lock Band List"))
	-- if (string.gsub(luci.sys.exec('uci get system.@system[0].modem |grep lte |wc -l'),"%s+","")=="1") then
	-- bandlist.default = "0"
	-- bandlist:value("1", "LTE BAND1")
	-- bandlist:value("2", "LTE BAND2")
	-- bandlist:value("3", "LTE BAND3")
	-- bandlist:value("4", "LTE BAND4")
	-- bandlist:value("5", "LTE BAND5")
	-- bandlist:value("7", "LTE BAND7")
	-- bandlist:value("8", "LTE BAND8")
	-- bandlist:value("20", "LTE BAND20")
	-- bandlist:value("38", "LTE BAND38")
	-- bandlist:value("40", "LTE BAND40")
	-- bandlist:value("41", "LTE BAND41")
	-- bandlist:value("28", "LTE BAND28")
	-- bandlist:value("A", "AUTO")
	-- end
	bandlist:value("0", translate("Disable"))

	servertype = section:taboption("advanced", ListValue, "servertype", translate("Server Type"))
	servertype.default = "0"

	--if (string.gsub(luci.sys.exec('uci get system.@system[0].modem |grep nr5g |wc -l'),"%s+","")=="1") then
		servertype:value("1", "5G Only")
		servertype:value("5", "4G/5G Only")
	--end
	servertype:value("2", "4G Only")
	servertype:value("3", "3G Only")
	servertype:value("4", "2G Only")
	servertype:value("0", "AUTO")


	-- s1 = m:section(TypedSection, "ndis", translate("AT Port Settings"),translate("Set tyyUSB port"))
	-- s1.anonymous = true
	-- s1.addremove = false
	-- tyyusb= s1:option(Value, "tyyusb", translate("tyyUSB port"))
	-- tyyusb.default = "2"
	-- tyyusb:value("0", "0")
	-- tyyusb:value("1", "1")
	-- tyyusb:value("2", "2")
	-- tyyusb:value("3", "3")
	-- tyyusb:value("4", "4")
	-- tyyusb.rmempty=false
end









s2 = m:section(TypedSection, "ndis", translate("Network Diagnostics"),translate("Network exception handling: \
check the network connection in a loop for 5 seconds. If the Ping IP address is not successful, After the network \
exceeds the abnormal number, restart and search the registered network again."))
s2.anonymous = true
s2.addremove = false

en = s2:option(Flag, "en", translate("Enable"))
en.rmempty = false



ipaddress= s2:option(Value, "ipaddress", translate("Ping IP address"))
ipaddress.default = "114.114.114.114"
ipaddress.rmempty=false

an = s2:option(Value, "an", translate("Abnormal number"))
an.default = "15"
an:value("3", "3")
an:value("5", "5")
an:value("10", "10")
an:value("15", "15")
an:value("20", "20")
an:value("25", "25")
an:value("30", "30")
an.rmempty=false



local apply = luci.http.formvalue("cbi.apply")
if apply then
    -- io.popen("/etc/init.d/modeminit restart")
	io.popen("/etc/init.d/modem restart")
end

return m,m2
