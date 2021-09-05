module "luci.controller.olsr-viz"

function index()
	entry({"admin", "status", "olsr", "olsr-viz"}, template("olsr-viz/olsr-viz"), _("OLSR-Viz"), 90)
end
