require("rv_rename")

local CallOperator = RV.CallOperator
local isVec = function(a)
	return a._postfix == "V"
end
VecC = {}
VecC.Postfix = "V"
VecC.IsVec = isVec
VecC.Metatable = {
	__add = function(a,b)
		assert(a.IsVec(b))
		return a:luaAddV(b)
	end,
	__sub = function(a,b)
		assert(a.IsVec(b))
		return a:luaSubV(b)
	end,
	__mul = function(a,b)
		return CallOperator(a,b, "luaMul")
	end,
	__div = function(a,b)
		return CallOperator(a,b, "luaDiv")
	end,
	__unm = function(a)
		return a:luaInvert()
	end,
	__len = function(a)
		return a:length()
	end,
	__eq = function(a,b)
		return a:luaEqual(b)
	end,
	__tostring = function(a)
		return a:luaToString()
	end
}
