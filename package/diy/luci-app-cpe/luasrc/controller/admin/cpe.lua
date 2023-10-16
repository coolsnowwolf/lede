module("luci.controller.admin.cpe", package.seeall) 

I18N = require "luci.i18n"
translate = I18N.translate

function index()
	entry({"admin", "modem"}, firstchild(), translate("移动数据"), 25).dependent=false
	entry({"admin", "modem", "nets"}, template("cpe/net_status"), translate("信号状态"), 0)
	entry({"admin", "modem", "get_csq"}, call("action_get_csq"))
	entry({"admin", "modem", "send_atcmd"}, call("action_send_atcmd"))

	-- entry({"admin", "modem", "sms"}, template("cpe/sms"), translate("短信信息"), 1)
	-- entry({"admin", "modem", "band"}, template("cpe/band"), translate("锁频段/锁PCI"), 1)
	entry({"admin", "modem", "at"}, template("cpe/at"), translate("AT工具"), 98)
	
	if not nixio.fs.access("/etc/config/modem") then
		return
	end
	entry({"admin", "modem", "modem"}, cbi("cpe/modem"), _("模块设置"), 99) 
	
end

function action_send_atcmd()
	local rv ={}
	local file
	local p = luci.http.formvalue("p")
	local set = luci.http.formvalue("set")
	fixed = string.gsub(set, "\"", "~")
	port= string.gsub(p, "\"", "~")
	rv["at"] = fixed 
	rv["port"] = port

	os.execute("/usr/share/cpe/atcmd.sh \'" .. port .. "\' \'" .. fixed .. "\'")
	result = "/tmp/result.at"
	file = io.open(result, "r")
	if file ~= nil then
		rv["result"] = file:read("*all")
		file:close()
	else
		rv["result"] = " "
	end
	os.execute("/usr/share/cpe/delatcmd.sh")
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)

end

function action_get_csq()
	local file
	stat = "/tmp/cpe_cell.file"
	file = io.open(stat, "r")
	local rv ={}

	-- echo 'RM520N-GL'
	-- echo 'conntype'
	-- echo '1e0e:9001'
	-- echo $COPS #运营商
	-- echo '' #端口
	-- echo '' #温度
	-- echo '' #协议 
    rv["modem"] = file:read("*line")
	rv["conntype"] = file:read("*line")
	rv["modid"] = file:read("*line")
	rv["cops"] = file:read("*line")
	rv["port"] = file:read("*line")
	rv["tempur"] = file:read("*line")
	rv["proto"] = file:read("*line")
	file:read("*line")


	-- echo $IMEI #imei
	-- echo $IMSI #imsi
	-- echo $ICCID #iccid
	-- echo $phone #phone
	rv["imei"] = file:read("*line")
	rv["imsi"] = file:read("*line")
	rv["iccid"] =file:read("*line")
	rv["phone"] = file:read("*line")
	file:read("*line")


	-- echo $MODE
	-- echo $CSQ
	-- echo $CSQ_PER
	-- echo $CSQ_RSSI
	-- echo '' #参考信号接收质量 RSRQ ecio
	-- echo '' #参考信号接收质量 RSRQ ecio1
	-- echo '' #参考信号接收功率 RSRP rscp
	-- echo '' #参考信号接收功率 RSRP rscp1
	-- echo '' #信噪比 SINR  rv["sinr"]
	-- echo '' #连接状态监控 rv["netmode"]
	rv["mode"] = file:read("*line")
	rv["csq"] = file:read("*line")
	rv["per"] = file:read("*line")
	rv["rssi"] = file:read("*line")
	rv["ecio"] = file:read("*line")
	rv["ecio1"] = file:read("*line")
	rv["rscp"] = file:read("*line")
	rv["rscp1"] = file:read("*line")
	rv["sinr"] = file:read("*line")
	rv["netmode"] = file:read("*line")
	file:read("*line")
	
	rssi = rv["rssi"]
	ecio = rv["ecio"]
	rscp = rv["rscp"]
	ecio1 = rv["ecio1"]
	rscp1 = rv["rscp1"]
	if ecio == nil then
		ecio = "-"
	end
	if ecio1 == nil then
		ecio1 = "-"
	end
	if rscp == nil then
		rscp = "-"
	end
	if rscp1 == nil then
		rscp1 = "-"
	end

	if ecio ~= "-" then
		rv["ecio"] = ecio .. " dB"
	end
	if rscp ~= "-" then
		rv["rscp"] = rscp .. " dBm"
	end
	if ecio1 ~= " " then
		rv["ecio1"] = " (" .. ecio1 .. " dB)"
	end
	if rscp1 ~= " " then
		rv["rscp1"] = " (" .. rscp1 .. " dBm)"
	end

	rv["mcc"] = file:read("*line")
	rv["mnc"] = file:read("*line")
    rv["rnc"] = file:read("*line")
	rv["rncn"] = file:read("*line")
	rv["lac"] = file:read("*line")
	rv["lacn"] = file:read("*line")
	rv["cid"] = file:read("*line")
	rv["cidn"] = file:read("*line")
	rv["lband"] = file:read("*line")
	rv["channel"] = file:read("*line")
	rv["pci"] = file:read("*line")

	rv["date"] = file:read("*line")
	
	-- rv["phonen"] = file:read("*line")
	--rv["host"] = "0"

	-- rv["simerr"] = "0"
	-- 
	-- 
	-- 
	-- 
	-- 
	-- 
	-- 
	-- 
	-- 
	-- 

	
	-- rv["down"] = file:read("*line")
	-- rv["up"] = file:read("*line")
	-- 
	-- 

	-- 
	-- rv["cell"] = file:read("*line")
	-- rv["modtype"] = file:read("*line")
	-- 
	-- 
	-- 
	-- 
	-- 
	-- 


	-- rv["lat"] = "-"
	-- rv["long"] = "-"	



	rv["crate"] = translate("快速(每10秒更新一次)")
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end