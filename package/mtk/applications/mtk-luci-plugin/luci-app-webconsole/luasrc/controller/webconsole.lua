-- A simple web console in case you don't have access to the shell
-- 
-- Hua Shao <nossiac@163.com>

module("luci.controller.webconsole", package.seeall)
local http = require("luci.http")
function index()
    entry({"admin", "mtk", "console"}, template("mtk_web_console"), _("Web Console"), 4)
    entry({"admin", "mtk", "webcmd"}, call("webcmd"))
end

function webcmd()
    local cmd = http.formvalue("cmd")
    if cmd then
	    local fp = io.popen(tostring(cmd).." 2>&1")
	    local result =  fp:read("*a")
	    fp:close()
        result = result:gsub("<", "&lt;")
        http.write(tostring(result))
    else
        http.write_json(http.formvalue())
    end
end
