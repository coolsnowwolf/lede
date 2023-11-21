module("luci.controller.admin.modemtools", package.seeall) 

I18N = require "luci.i18n"
translate = I18N.translate

function index()
    entry({"admin", "modemtools"}, firstchild(), "ModemTools", 25).dependent=false
    entry({"admin", "modemtools", "send_atcmd"}, call("action_send_atcmd"))
    entry({"admin", "modemtools", "attools"}, template("modemtools/attools"), translate("AT工具"), 98)
end

function action_send_atcmd()
    local res ={}
    local command = luci.http.formvalue("command")
    local device = luci.http.formvalue("device")

    local atfd = io.popen("COMMAND=".. command  .." gcom -d ".. device .." -s /etc/gcom/getatresponse.gcom")
    if nil == atfd then
        res["response"] = " "
    else
        res["response"] = atfd:read("*a")
        atfd:close()
    end

    luci.http.prepare_content("application/json")
    luci.http.write_json(res)
end
