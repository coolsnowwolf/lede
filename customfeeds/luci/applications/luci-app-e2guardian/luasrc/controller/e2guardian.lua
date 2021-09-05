--[[

LuCI E2Guardian module

Copyright (C) 2015, Itus Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Author: Marko Ratkaj <marko.ratkaj@sartura.hr>
	Luka Perkov <luka.perkov@sartura.hr>

]]--

module("luci.controller.e2guardian", package.seeall)

function index()
	entry({"admin", "services", "e2guardian"}, cbi("e2guardian"), _("E2Guardian"))
end
