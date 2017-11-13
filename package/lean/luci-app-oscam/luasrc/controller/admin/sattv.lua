module("luci.controller.admin.sattv", package.seeall)

function index()
	
--	entry({"admin", "Others"}, cbi("sattv/epg"), i18n("Others"), 70)
		
	if nixio.fs.access("/etc/config/epg") then
	 entry({"admin", "Others"}, cbi("sattv/epg"), _("sattv"), 66).index = true
   	 entry({"admin", "Others", "epg"}, cbi("sattv/epg"), _("EPG"), 1).i18n = "diskapply"
	end
	
  if nixio.fs.access("/etc/config/oscam") then
	  entry({"admin", "Others", "oscam"}, cbi("sattv/oscam"), _("OSCAM"), 12).i18n = "diskapply"
	end

end
