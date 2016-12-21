if RV == nil then
	RV = {}
end

local rawget, rawset = rawget, rawset
local pairs = pairs
local type = type
local tostring = tostring
function RV.Concat(src0, src1)
	local n = #src0
	for i=1,#src1 do
		n = n+1
		src0[n] = src1[i]
	end
end
function RV.Join(dst, src)
	for k,v in pairs(src) do
		local v = dst[k]
		if v == nil then
			dst[k] = v
		end
	end
end
function RV.JoinRaw(dst, src)
	for k,v in pairs(src) do
		local v = rawget(dst, k)
		if v == nil then
			rawset(dst, k, v)
		end
	end
end
function RV.Overwrite(dst, src)
	for k,v in pairs(src) do
		dst[k] = v
	end
end
function RV.OverwriteRaw(dst, src)
	for k,v in pairs(src) do
		rawset(dst, k, v)
	end
end
