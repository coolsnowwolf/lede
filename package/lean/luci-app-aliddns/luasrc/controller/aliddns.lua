module("luci.controller.aliddns",package.seeall)
function index()
entry({"admin","services","aliddns"},cbi("aliddns"),_("AliDDNS"),58)
end
