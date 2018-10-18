require("rv_rename")

local CallOperator = RV.CallOperator
local isDegree = function(a)
	return a._postfix == "D"
end
return {
	IsDegree = isDegree,
	_postfix = "D",
	_metatable = {
		__add = function(a,b)
			return CallOperator(a,b, "luaAdd")
		end,
		__sub = function(a,b)
			return CallOperator(a,b, "luaSub")
		end,
		__mul = function(a,b)
			assert(type(b) == "number")
			return a:luaMulF(b)
		end,
		__div = function(a,b)
			assert(type(b) == "number")
			return a:luaDivF(b)
		end,
		__unm = function(a)
			return a:luaInvert()
		end,
		__lt = function(a,b)
			return a:luaLessthan(b:toDegree())
		end,
		__le = function(a,b)
			return a:luaLessequal(b:toDegree())
		end,
		__eq = function(a,b)
			return a:luaEqual(b:toDegree())
		end,
		__tostring = function(a)
			return a:luaToString()
		end
	},
	_renamefunc =
		RV.RenameFunc(
			{"toDegree", "luaToDegree"},
			{"toRadian", "luaToRadian"}
		)
}
