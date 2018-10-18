require("vector_common")

local CallOperator = RV.CallOperator
local isMat = function(a)
	return a._postfix == "M"
end
MatC = {}
MatC.Postfix = "M"
MatC.Metatable = {
	IsMat = isMat,
	__add = function(a,b)
		assert(VecC.IsVec(b))
		return a:luaAddV(b)
	end,
	__sub = function(a,b)
		assert(VecC.IsVec(b))
		return a:luaSubV(b)
	end,
	__mul = function(a,b)
		return CallOperator(a,b, "luaMul")
	end,
	__div = function(a,b)
		assert(type(b) == "number")
		return a:luaDivF(b)
	end,
	__unm = function(a)
		return a:luaInvert()
	end,
	__eq = function(a,b)
		return a:luaEqual(b)
	end,
	__tostring = function(a)
		return a:luaToString()
	end
}
