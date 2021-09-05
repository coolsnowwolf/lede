local s = require"luasql.postgres"

local env = assert (luasql.postgres ())
local conn = assert (env:connect ("luasql-test", "tomas"))
local cur = assert (conn:execute ("select count(*) from fetch_test"))

cgilua.htmlheader()
cgilua.put ("Total lines at table fetch_test is "..cur:fetch())
cgilua.put (string.format ("<br>\n%s == %s<br>\n", tostring(s), tostring(luasql)))

cur:close()
conn:close()
env:close()
