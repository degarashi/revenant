require("vector_common")
require("rv_rename")

local tmp = {
	_size = 2,
	IsVec = function(a)
		return VecC.IsVec(a) and a._size == 2
	end,
	_postfix = VecC.Postfix,
	_metatable = VecC.Metatable,
	_renamefunc =
		RV.RenameFunc(
			{"toVec3", "convert<3>"},
			{"toVec4", "convert<4>"}
		)
}
return tmp
