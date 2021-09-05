require"postgres"

local env = assert (luasql.postgres ())
local conn = assert (env:connect ("luasql-test", "tomas"))
local cur = assert (conn:execute ("select count(*) from t1"))
local total = tonumber (cur:fetch())
cur:close()
local aleatorio = math.random(total)
local cur = assert (conn:execute ("select * from t1, t2 where b = d and a != "..2*aleatorio))

cgilua.htmlheader()
cgilua.put ("Aleatorio = "..aleatorio.."<br>\n")

local a,b,c,d = cur:fetch()
cgilua.put ("<table>\n")
while a do
--	cgilua.put ("<tr><td>",a,"<td>",b,"<td>",c,"<td>",d,"</tr>")
	a,b,c,d = cur:fetch()
end
cgilua.put ("</table>\n")

cur:close()
conn:close()
env:close()
