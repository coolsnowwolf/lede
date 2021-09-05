-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.statistics.i18n", package.seeall)

require("luci.util")
require("luci.i18n")


Instance = luci.util.class()


function Instance.__init__( self, graph )
	self.i18n  = luci.i18n
	self.graph = graph
end

function Instance._subst( self, str, val )
	str = str:gsub( "%%H",  self.graph.opts.host or "" )
	str = str:gsub( "%%pn", val.plugin or "" )
	str = str:gsub( "%%pi", val.pinst  or "" )
	str = str:gsub( "%%dt", val.dtype  or "" )
	str = str:gsub( "%%di", val.dinst  or "" )
	str = str:gsub( "%%ds", val.dsrc   or "" )

	return str
end

function Instance.title( self, plugin, pinst, dtype, dinst, user_title )

	local title = user_title or
		"p=%s/pi=%s/dt=%s/di=%s" % {
			plugin,
			(pinst and #pinst > 0) and pinst or "(nil)",
			(dtype and #dtype > 0) and dtype or "(nil)",
			(dinst and #dinst > 0) and dinst or "(nil)"
		}

	return self:_subst( title, {
		plugin = plugin,
		pinst  = pinst,
		dtype  = dtype,
		dinst  = dinst
	} )

end

function Instance.label( self, plugin, pinst, dtype, dinst, user_label )

	local label = user_label or
		"dt=%s/di=%s" % {
			(dtype and #dtype > 0) and dtype or "(nil)",
			(dinst and #dinst > 0) and dinst or "(nil)"
		}

	return self:_subst( label, {
		plugin = plugin,
		pinst  = pinst,
		dtype  = dtype,
		dinst  = dinst
	} )

end

function Instance.ds( self, source )

	local label = source.title or
		"dt=%s/di=%s/ds=%s" % {
			(source.type     and #source.type     > 0) and source.type     or "(nil)",
			(source.instance and #source.instance > 0) and source.instance or "(nil)",
			(source.ds       and #source.ds       > 0) and source.ds       or "(nil)"
		}

	return self:_subst( label, {
		dtype = source.type,
		dinst = source.instance,
		dsrc  = source.ds
	} ):gsub(":", "\\:")

end
