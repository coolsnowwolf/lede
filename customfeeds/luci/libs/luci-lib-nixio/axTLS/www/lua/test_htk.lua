require"htk"

local a_table = {}
for i = 1, 20 do
	local l = {}
	for j = 1, 20 do
		table.insert (l, HTK.TD { "cell "..i..","..j })
	end
	table.insert (a_table, HTK.TR (l))
end

cgilua.htmlheader()
cgilua.put (HTK.HTML {
	HTK.HEAD { HTK.TITLE { "Titulo da Pagina" } },
	HTK.BODY {
		bgcolor = "#FFFFFF",
		HTK.H1 { "Titulo da Pagina" },
		HTK.P {},
		"Uma página qualquer",
		HTK.TABLE (a_table),
	}
})
