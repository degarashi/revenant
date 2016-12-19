-- Luaとstd::shared_ptr, std::weak_ptrをやり取りするための関数群
Void = {
	handleName = "void"
}
Void.mt = {
	__index = Void
}
Void_w = {
	handleName = "void"
	-- lockメソッドなど
}
Void_w.mt = {
	__index = Void_w
}

local mt_v = {
	__mode = "v"
}
-- shared_ptr保持リスト(弱参照)
-- LightUserdata(void*) -> Userdata(shared_ptr)
local PtrToSP = {}
setmetatable(PtrToSP, mt_v)

-- 既にSPインスタンスがLua上に生成されていればそれを返す
function HasSP(ptr)
	return PtrToSP[ptr]
end
-- 新たにハンドルを作成
function MakeSP(ptr, name, sp)
	assert(PtrToSP[ptr] == nil)
	local h = { udata = sp }
	-- オブジェクト固有のメタテーブルを付加
	setmetatable(h, _G[name].mt)
	PtrToSP = h
	return h
end
function GetSP(h)
	if h == nil then
		return nil
	end
	return h.udata
end

-- weak_ptr保持リスト(弱参照)
-- LightUserdata(void*) -> Userdata(weak_ptr)
local PtrToWP = {}
setmetatable(PtrToWP, mt_v)

function HasWP(ptr)
	return PtrToWP[ptr]
end
function MakeWP(ptr, name, wp)
	assert(PtrToWP[ptr] == nil)
	local h = { udata = wp }
	-- オブジェクト固有のメタテーブルを付加
	setmetatable(h, _G[name .. "_w"].mt)
	PtrToWP[ptr] = h
	return h
end
function GetWP(h)
	if h == nil then
		return nil
	end
	return h.udata
end
