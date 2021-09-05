#!/usr/local/bin/lua

MAX_ROWS = arg[1] or 10

require"postgres"

local env = assert (luasql.postgres ())
local conn = assert (env:connect ("luasql-test", "tomas"))

-- Apaga restos de outros testes.
conn:execute "drop table t2"
conn:execute "drop table t1"

-- Criando as tabelas.
assert (conn:execute [[create table t1 (
	a int,
	b int
)]])
assert (conn:execute [[create table t2 (
	c int,
	d int
)]])

-- Preenchedo as tabelas.
for i = 1, MAX_ROWS do
	local ii = 2*i
	assert (conn:execute (string.format ([[
insert into t1 values (%d, %d);
insert into t2 values (%d, %d);]],
		ii, i, ii, i)))
end
