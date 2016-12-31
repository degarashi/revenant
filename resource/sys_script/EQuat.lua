require("rv_rename")

local CallOperator = RV.CallOperator
local isEQuat = function(a)
	return a._postfix == "E"
end
return {
	IsEQuat = isEQuat,
	_postfix = "E",
	_metatable = {
	}
}
