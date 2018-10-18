require("object")

do
	-- テーブル継承に使うメタテーブルを作成
	-- 読み取りは自テーブル、ベーステーブルの順
	-- 書き込みは両方が持って無ければ自分に書き込む
	local makeDerivedTableMT = function(base)
		return {
			__index = base,
			__newindex = function(tbl, key, val)
				if base[key] ~= nil then
					base[key] = val
				else
					rawset(tbl, key, val)
				end
			end
		}
	end
	-- FSMachineで使うステート継承
	function DerivedState(base, state)
		state = state or {}
		state._base = base
		setmetatable(state, makeDerivedTableMT(base))
		return state
	end
end
--[[
	FSMachine基底を定義(from C++)
	ステート遷移ベースクラス
	self.valueとした場合はインスタンス変数, static変数, c++変数の順で参照
	RecvMsgでは[現ステート, Baseステート, クラスstatic(C++メソッド)]の順でチェックする
]]
local upd_obj = Object
-- この時点でベースとなる"Object"が定義されていなければならない
assert(upd_obj)
FSMachine = DefineObject(upd_obj, "FSMachine", {
	--[[
		[変数説明]
		state(table)		: 現在のステート
		stateS(string)		: 現在のステート名
		nextStateS(string)	: 移行先のステート名
		nextParam(table)	: 次のステートに渡すパラメータ {...}
		stateLocal(table)	: ステートローカル領域
	]]
	Ctor = function(self, firstState, ...)
		upd_obj.Ctor()
		-- ステート管理変数の初期化
		assert(type(firstState) == "string")
		self.state = self[firstState]
		self.stateS = firstState
		self.stateLocal = {}
		self:RecvMsg("OnEnter", nil)
	end,
	-- \param[in] state(string)	次のステート名
	SetState = function(self, state, ...)
		-- 既に次のステートがセットされていたらエラー
		assert(not self.nextStateS)
		self.nextStateS = state
		self.nextParam = {...}
	end,
	_SwitchState = function(self)
		-- ステート変更が全て終わるまでループ
		while self.nextStateS do
			assert(type(self.nextStateS) == "string")

			local nextS = self.nextStateS
			self.nextStateS = nil
			self:RecvMsg("OnExit", nextS)

			-- stateLocalを一旦破棄
			self.stateLocal = {}

			local tmpParam = self.nextParam
			self.nextParam = nil
			local prevS = self.stateS
			self.state = self[nextS]
			self.stateS = nextS
			if self.state then
				self:RecvMsg("OnEnter", prevS, table.unpack(tmpParam))
			end
		end
	end,
	-- 全てのメッセージは先頭引数がself, lc(ステートローカル領域), 任意の引数... と続く
	-- OnEnter (prevState(string))
	-- OnExit (nextState(string))
	-- OnCollisionEnd (obj(object), nFrame(number))
	-- OnCollision (obj(object), nFrame(number))

	-- Lua -> C++の順でメッセージ受信を試みる
	--[[
		\param[in] msg(string)	メッセージ名
		\return (bool)受信応答, 任意の戻り値...
	]]
	RecvMsg = function(self, msg, ...)
		-- 現在のFSMachineのステートが受信できればそうする
		local proc = self.state[msg]
		if proc then
			local ret = {proc(self, self.stateLocal, ...)}
			-- OnExitでステート変更は不可
			assert(msg ~= "OnExit" or not self.nextStateS)
			self:_SwitchState()
			-- 返り値の前に受信応答(true)を付加する
			return true, table.unpack(ret)
		end
		-- C++のメソッド呼び出しとして処理
		local cppproc = self[msg]
		if type(cppproc) == "function" then
			return true, cppproc(self, ...)
		end
		-- C++の汎用メッセージとして処理
		return upd_obj.RecvMsg(self, msg, ...)
	end
})
