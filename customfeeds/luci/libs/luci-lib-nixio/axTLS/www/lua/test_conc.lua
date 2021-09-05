cgilua.htmlheader()
if ap then
	local pid, ppid = ap.pid ()
	if not ppid then
		ppid = "no parent pid"
	end
	cgilua.put ("pid = "..pid.." ("..ppid..")".."\n")
end

assert(type(stable.get) == "function")
assert(type(stable.set) == "function")

cgilua.put"stable.pairs = {<br>\n"
for i, v in stable.pairs () do
	cgilua.put (i.." = "..tostring(v).."<br>\n")
end
cgilua.put"}<br>\n"

local counter = stable.get"counter" or 0
stable.set ("counter", counter + 1)

local f = stable.get"f"
if not f then
	local d = os.date()
	stable.set ("f", function () return d end)
else
	cgilua.put ("f() = "..tostring (f ()))
end

cgilua.put"<br>\n"
for i = 1,800 do
	cgilua.put (i)
	for ii = 1,1000 do
		cgilua.put ("<!>")
	end
	cgilua.put ("\n")
end
cgilua.put ("End")
