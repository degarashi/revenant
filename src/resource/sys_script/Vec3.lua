require("vector_common")
require("sysfunc")
local tmp = {
	_size = 3,
	IsVec = function(a)
		return VecC.IsVec(a) and a._size == 3
	end,
	_postfix = VecC.Postfix,
	_metatable = VecC.Metatable,
	_renamefunc =
		RV.RenameFunc(
			{"toVec2", "convert<2>"},
			{"toVec4", "convert<4>"}
		)
}
return tmp
