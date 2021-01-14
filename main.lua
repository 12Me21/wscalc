local INFIX = {
	["+"]=function(a,b) return a+b end,
	["-"]=function(a,b) return a-b end,
	["*"]=function(a,b) return a*b end,
	["/"]=function(a,b) return a/b end,
	["%"]=function(a,b) return a%b end,
	["^"]=function(a,b) return a^b end,
	["~"]=function(a,b) return a//1~b//1 end,
	["&"]=function(a,b) return a//1&b//1 end,
	["|"]=function(a,b) return a//1|b//1 end,
	[">>"]=function(a,b) return a//1>>b//1 end,
	["<<"]=function(a,b) return a//1>>b//1 end,
}
local PREFIX = {
	["-"]=function(a) return -a end,
	["~"]=function(a) return ~(a//1) end,
}
function search(str, map)
	for k,v in pairs(map) do
		if str:sub(1,#k)==k then return k,v	end
	end
end

-- return: value, string
function readExpr(str, depth)
	local acc, str = readValue(str, depth)
	return readAfter(str, depth, acc)
end

-- return: value, string
function readAfter(str, depth, acc)
	if str:sub(1,1)==" " then -- end group
		str = str:sub(2)
		if depth>0 then return acc, str end
	end
	if str=="" then return acc, str end -- end

	local op, f = search(str, INFIX)
	if op then
		local v
		v, str = readValue(str:sub(#op+1), depth)
		return readAfter(str, depth, f(acc, v))
	end
	error("syntax error: "..str)
end

-- return: value, string
function readValue(str, depth)
	
	local num = str:match("^%d*%.?%d+")
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

function eval(exp, acc)
	local status, res = pcall(readAfter, exp, 0, acc)
	if status then return res end
	status, res = pcall(readExpr, exp, 0)
	if status then return res end
	print(res)
	return acc
end

local acc = 0
repeat
	local line = io.read()
	acc = eval(line, acc)
	print(">", acc)
until nil
