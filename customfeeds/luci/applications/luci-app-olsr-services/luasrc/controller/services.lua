module "luci.controller.services"

function index()
	local uci = require "luci.model.uci".cursor()

	uci:foreach("olsrd", "LoadPlugin", function(s)
		if s.library == "olsrd_nameservice.so.0.3" then
			has_serv = true
		end
	end)

	if has_serv then
		entry({"freifunk", "services"}, template("freifunk-services/services"), _("Services"), 60)
	end
end

