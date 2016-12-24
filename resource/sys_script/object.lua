require("shared_ptr")
require("rv_table")

local setmetatable = setmetatable
local getmetatable = getmetatable
local rawset,rawget = rawset,rawget
local type = type
local assert = assert
local print = print

-- ゲーム固有のグローバル変数は全てここに含める
Global = {}
-- ゲームエンジンに関する変数など
System = {}

do
	--[[
		クラス定義ファイルを読む前にこの関数でUpValueにセットする
		(クラス定義ファイル中でグローバル関数を読んだ場合への対処)
		Global = 明示的なグローバル変数テーブル
		System = システム関数テーブル
		RV = 自作エンジンの雑多な関数
		G = 本来のグローバル環境
	]]
	local global, system, g_tbl, rv_tbl = Global, System, _G, RV
	function MakePreENV(base)
		setmetatable(base, {
			__index =
				function(tbl, key)
					if key == "System" then
						return system
					elseif key == "Global" then
						return global
					elseif key == "G" then
						return g_tbl
					elseif key == "RV" then
						return rv_tbl
					end
				end
		})
	end
end

-- クラスのEnum値などを（あれば）読み込み
local _LoadAdditionalValue = function(modname)
	-- モジュールが見つからなかった場合は空のテーブルを返すようにする
	local ps = package.searchers
	ps[#ps+1] = function(...)
		return	function()
					return {}
				end
	end
	local ret = require(modname)
	-- 元に戻しておく
	ps[#ps] = nil
	return ret
end
-- C++クラスのメソッド、メンバ読み込み(メタメソッド)
function __Index(base, tbl, key)
	local r = rawget(base, key)
	if r ~= nil then
		return r
	end
	-- メンバ変数(C++)読み込み
	r = base._valueR[key]
	if r~=nil then
		return r(tbl)
	end
	-- メンバ関数(C++)呼び出し
	r = base._func[key]
	if r~=nil then
		return r
	end
	-- ベースクラス読み込み
	base = base._base
	if base then
		return __Index(base, tbl, key)
	end
end
-- C++クラスのメンバ書き込み(メタメソッド)
function __NewIndex(base, tbl, key, val)
	local w = rawget(base, key)
	if w~=nil then
		rawset(base, key, val)
		return
	end
	-- メンバ変数(C++)書き込み
	w = base._valueW[key]
	if w~=nil then
		w(tbl, val)
		return
	end
	rawset(tbl, key, val)
end
-- オブジェクト型定義
--[[
	\param[in]	base(table)		ベースクラスまたは事前にC++で定義しておいたObject
	\param[in]	name(string)	クラス名
	-- C++で事前に定義しておいたクラスの例 --
	Object {
		_valueR = {NumRef(func)}
		_valueW = {}
		_func = {}
		handleName = "ObjectName"
		RecvMsg(func) = RecvMsgCpp
		RecvMsgCpp(func)
		Ctor(func)

		_new = cfunc(...)
	}
]]
function DefineObject(base, name, object, bNoLoadValue)
	assert(base, "DefineObject: base-class is nil")
	assert(type(name)=="string", "DefineObject: invalid ObjectName")
	assert(object==nil or type(object)=="table", "DefineObject: invalid argument (object)")

	if _G[name] ~= nil then
		assert(object ~= nil)
		return object
	end
	if getmetatable(base) == nil then
		MakePreENV(base)
	end

	object = object or {}
	local _r, _w, _f = {},{},{}
	-- インスタンスにセットするMT
	local _mt = {
		__index = function(tbl, key)
			return __Index(object, tbl, key)
		end,
		__newindex = function(tbl, key, val)
			__NewIndex(object, tbl, key, val)
		end
	}
	-- 付加情報ファイルがあったら読み込み
	if not bNoLoadValue then
		RV.Overwrite(object, _LoadAdditionalValue(name))
		local mt = object._metatable
		if mt then
			RV.Overwrite(_mt, mt)
		end
		local rf = object._renamefunc
		if rf then
			RV.Overwrite(_f, rf)
		end
		object._valueR = _r
		object._valueW = _w
		object._func = _f
		object._new = false		-- 後で(C++にて)定義する用のエントリー確保ダミー(メタテーブルの関係)
	end
	if object.Ctor==nil then
		-- [Protected] 空のコンストラクタを用意
		function object.Ctor(self, ...)
			print("object.default Ctor:" .. object._name)
			base.Ctor(self, ...)
		end
	end
	object._name = name
	object._base = base
	object._metatable = _mt

	-- [Public] ポインタからオブジェクトを構築 (from C++)
	--[[ \param[in] ud(userdata) ]]
	function object.ConstructPtr(ud)
		-- LightUserDataでは個別のmetatableを持てないのでtableで包む
		local ret = { _pointer = ud }
		setmetatable(ret, _mt)
		return ret
	end
	-- [Public] インスタンス作成(from Lua & C++)
	function object.New(...)
		-- C++側で定義されたインスタンス確保関数を呼ぶ -> SPtr
		local obj = object._new(...)
		-- インスタンス用メタテーブルをセットした後、
		setmetatable(obj, _mt)
		-- Lua側で定義されたコンストラクタを呼ぶ
		obj:Ctor(...)
		return obj
	end
	-- staticな変数を取得、代入する為のメタテーブル
	setmetatable(object, {
		__index = function(tbl, key)
			local base = tbl._base
			if base then
				return base[key]
			end
		end,
		__newindex = function(tbl, key, val)
			local r = base[key]
			if r ~= nil then
				base[key] = val
				return
			end
			rawset(tbl, key, val)
		end
	})
	return object
end
