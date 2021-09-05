-------------------------------------------------------------------------------
-- LuaDoc configuration file. This file contains the default options for 
-- luadoc operation. These options can be overriden by the command line tool
-- @see luadoc.print_help
-- @release $Id: config.lua,v 1.6 2007/04/18 14:28:39 tomas Exp $
-------------------------------------------------------------------------------

module "luadoc.config"

-------------------------------------------------------------------------------
-- Default options
-- @class table
-- @name default_options
-- @field output_dir default output directory for generated documentation, used
-- by several doclets
-- @field taglet parser used to analyze source code input
-- @field doclet documentation generator
-- @field template_dir directory with documentation templates, used by the html
-- doclet
-- @field verbose command line tool configuration to output processing 
-- information

local default_options = {
	output_dir = "",
	taglet = "luadoc.taglet.standard",
	doclet = "luadoc.doclet.html",
	-- TODO: find a way to define doclet specific options
	template_dir = "luadoc/doclet/html/",
	nomodules = false,
	nofiles = false,
	verbose = true,
}

return default_options
