require("vector_common")
local tmp = {
	_size = 4,
	IsVec = function(a)
		return VecC.IsVec(a) and a._size == 4
	end,
	_postfix = VecC.Postfix,
	_metatable = VecC.Metatable
}
return tmp
