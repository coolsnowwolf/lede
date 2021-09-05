-----------------------------------------------------------------
-- LuaDoc debugging facilities.
-- @release $Id: debug.lua,v 1.3 2007/04/18 14:28:39 tomas Exp $
-----------------------------------------------------------------

module "luadoc.doclet.debug"

function printline()
	print(string.rep('-', 79))
end

-----------------------------------------------------------------
-- Print debug information about document
-- @param doc Table with the structured documentation.

function start (doc)
	print("Files:")
	for _, filepath in ipairs(doc.files) do
		print('\t', filepath)
	end
	printline()

	print("Modules:")
	for _, modulename in ipairs(doc.modules) do
		print('\t', modulename)
	end
	printline()
	
	for i, v in pairs(doc.files) do
		print('\t', i, v)
	end
	printline()
	for i, v in pairs(doc.files[doc.files[1]]) do
		print(i, v)
	end
	
	printline()
	for i, v in pairs(doc.files[doc.files[1]].doc[1]) do
		print(i, v)
	end
	printline()
	print("Params")
	for i, v in pairs(doc.files[doc.files[1]].doc[1].param) do
		print(i, v)
	end
end
