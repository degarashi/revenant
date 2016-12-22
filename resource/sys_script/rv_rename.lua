-- システム関数はRVに纏める
if RV == nil then
	RV = {}
end

function RV.CallOperator(a, b, op)
	local t,postfix = type(b)
	if t == "table" then
		postfix = b._postfix
	else
		assert(t == "number")
		postfix = "F"
	end
	return a[op .. postfix](a,b)
end
function RV.RenameFuncJoin(tbl, entry, ...)
	if not entry then
		return
	end
	tbl[entry[1]] = function(self, ...)
		return self[entry[2]](self, ...)
	end
	RV.RenameFuncJoin(tbl, ...)
end
function RV.RenameFunc(entry, ...)
	if not entry then
		return
	end
	local tbl = {}
	RV.RenameFuncJoin(tbl, entry, ...)
	return tbl
end
local G = _G
function RV.RenameFuncStatic(tbl, tblname, entry, ...)
	if not entry then
		return
	end
	tbl[entry[1]] = function(...)
		return G[tblname][entry[2]](...)
	end
	RV.RenameFuncStatic(tbl, tblname, ...)
end
