local INFIX = {
	["+"]=function(a,b) return a+b end,
	["-"]=function(a,b) return a-b end,
}
local PREFIX = {
	["-"]=function(a) return -a end,
}

function readExpr(str)
	local acc, str = readValue(str)
	::found::
	if str:sub(1,1)==")" then
		return acc, str:sub(2)
	end
	if str=="" then
		return acc, str
	end
	for op,f in pairs(INFIX) do
		if str:sub(1,#op)==op then
			local v
			v, str = readValue(str:sub(#op+1))
			acc = f(acc, v)
			goto found
		end
	end
	return nil, str
end

function readValue(str)
	local num = str:match("^%d+")
	if num then
		return tonumber(num), str:sub(#num+1)
	end
	if str:sub(1,1) == "(" then
		return readExpr(str:sub(2))
	end
	for op,f in pairs(PREFIX) do
		if str:sub(1,#op)==op then
			local v
			v,str = readValue(str:sub(#op+1))
			return f(v), str
		end
	end
	return nil
end

function readValue2(str)
	for op,f in pairs(PREFIX) do
		if str:sub(1,#op)==op then
			local v, str2 = readValue(str:sub(#op+1))
			return f(v), str2
		end
	end
	local num = str:match("%d+")
	if num then
		str=str:sub(#num+1)
		local acc = tonumber(num)
		::found::
		for op,f in pairs(INFIX) do
			if str:sub(1,#op)==op then
				local v
				v, str = readValue(str:sub(#op+1))
				acc = f(acc, v)
				goto found
			end
		end
		if str:sub(1,1) == ")" then
			return acc, str:sub(2)
		end
		if str=="" then
			return acc,str
		end
		return nil, str
	end
	if str:sub(1,1)=="(" then
		return readValue(str:sub(2))
	end
	return nil, str
end

print(readExpr(arg[1]))
