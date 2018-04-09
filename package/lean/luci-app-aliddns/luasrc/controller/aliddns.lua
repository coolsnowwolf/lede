module("luci.controller.aliddns",package.seeall)
function index()
entry({"admin","services","aliddns"},cbi("aliddns"),_("Aliddns Client"),101)
end
