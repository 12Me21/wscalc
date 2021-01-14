local INFIX = {
	["+"]=function(a,b) return a+b end,
	["-"]=function(a,b) return a-b end,
	["*"]=function(a,b) return a*b end,
	["/"]=function(a,b) return a/b end,
}
local PREFIX = {
	["-"]=function(a) return -a end,
}
function search(str, map)
	for k,v in pairs(map) do
		if str:sub(1,#k)==k then return k,v	end
	end
end

function readExpr(str, depth)
	local acc, str = readValue(str, depth)

	::again::
	if str:sub(1,1)==" " then -- end group
		str = str:sub(2)
		if depth>0 then return acc, str end
	end
	if str=="" then return acc, str end -- end

	local op, f = search(str, INFIX)
	if op then
		local v
		v, str = readValue(str:sub(#op+1), depth)
		acc = f(acc, v)
		goto again
	end
	error("syntax error: "..str)
end

function readValue(str, depth)

	local num = str:match("^%d+")
	if num then
		return tonumber(num), str:sub(#num+1)
	end

	if str:sub(1,1) == " " then -- start group
		return readExpr(str:sub(2), depth+1)
	end

	local op, f = search(str, PREFIX)
	if op then
		local v
		v, str = readValue(str:sub(#op+1), depth)
		return f(v), str
	end
	error("syntax error: "..str)
end

print(readExpr(arg[1], 0))
