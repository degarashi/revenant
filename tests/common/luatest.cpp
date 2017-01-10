#include "../luatest.hpp"

namespace rev {
	namespace test {
		const LuaType c_luaTypes[] = {
			LuaType::Nil,
			LuaType::Number,
			LuaType::Boolean,
			LuaType::String,
			LuaType::Table,
			LuaType::Function,
			LuaType::Thread,
			LuaType::LightUserdata
		};
		const LuaType c_luaTypes_nonil[] = {
			LuaType::Number,
			LuaType::Boolean,
			LuaType::String,
			LuaType::Table,
			LuaType::Function,
			LuaType::Thread,
			LuaType::LightUserdata
		};
		const LuaType c_luaTypes_key[] = {
			LuaType::Number,
			LuaType::Boolean,
			LuaType::String,
			LuaType::Function,
			LuaType::Thread,
			LuaType::LightUserdata
		};

		#define PP_CFUNC(z,n,data) &CFunction::BOOST_PP_CAT(data,n),
		lua_CFunction CFunction::cs_func[N_Func] = {
			BOOST_PP_REPEAT(NUM_CFUNC, PP_CFUNC, Func)
		};
		#undef PP_CFUNC

		// --------------- LuaTest ---------------
		LuaTest::LuaTest():
			_lsp(LuaState::NewState()),
			_rdi(mt().template getUniformF<int>())
		{}
		lua_State* LuaTest::getLS() {
			return _lsp->getLS();
		}
		const Lua_SP& LuaTest::getLSP() {
			return _lsp;
		}
		const typename LuaTest::RdI& LuaTest::getRDI() const {
			return _rdi;
		}

		// --------------- GenValue ---------------
		LuaNil GenValue<LuaNil>::operator()(LuaTest&) const {
			return LuaNil{};
		}
		bool GenValue<bool>::operator()(LuaTest& self) const {
			return self._rdi({0,1});
		}
		void_sp GenValue<void_sp>::operator()(LuaTest& self) const {
			return GenValue_t<std::shared_ptr<int>>()(self);
		}
		void_wp GenValue<void_wp>::operator()(LuaTest& self) const {
			return GenValue_t<std::weak_ptr<int>>()(self);
		}
		lua_CFunction GenValue<lua_CFunction>::operator()(LuaTest& self) const {
			// 予め用意したC関数リストから一つを選ぶ
			return CFunction::cs_func[self._rdi({0, CFunction::N_Func-1})];
		}
		std::string GenValue<std::string>::operator()(LuaTest& self) const {
			return lubee::random::GenAlphabetString(self._rdi, self._rdi({0,32}));
		}
		void* GenValue<void*>::operator()(LuaTest& self) const {
			// ランダムなポインタを作成 -> LightUserDataなので参照されない
			return reinterpret_cast<void*>(self.mt().template getUniform<uintptr_t>());
		}
		Lua_SP GenValue<Lua_SP>::operator()(LuaTest& self) const {
			return self._lsp->newThread();
		}
		const char* GenValue<const char*>::operator()(LuaTest& self) const {
			self._stringVec.resize(self._stringVec.size()+1);
			self._stringVec.back() = GenValue_t<std::string>()(self);
			return self._stringVec.back().c_str();
		}
		lua_State* GenValue<lua_State*>::operator()(LuaTest& self) const {
			self._lsVec.emplace_back(self._lsp->newThread());
			return self._lsVec.back()->getLS();
		}
		LCValue GenValue<LCValue>::operator()(LuaTest& self) const {
			return self.genLCValue(c_luaTypes);
		}
		LCTable_SP GenValue<LCTable_SP>::operator()(LuaTest& self) const {
			auto ret = std::make_shared<LCTable>();
			// 配列エントリの初期化
			const int maxEnt = self._maxTableEnt;
			self._maxTableEnt /= 2;
			const int nAr = self._rdi({0, maxEnt});
			for(int i=0 ; i<nAr ; i++) {
				self.genLuaValue(c_luaTypes_nonil, [&ret, i](auto&& val){
					ret->emplace(LCValue(static_cast<lua_Number>(i)), LCValue(val));
				});
			}
			// 連想配列エントリの初期化
			const int nRec = self._rdi({0, maxEnt});
			for(int i=0 ; i<nRec ; i++) {
				for(;;) {
					// キーはテーブル以外
					LCValue key = self.genLCValue(c_luaTypes_key);
					// 既に同じキーを持っている場合は生成しなおし
					if(ret->count(key) == 0) {
						ret->emplace(key, self.genLCValue(c_luaTypes_nonil));
						break;
					}
				}
			}
			return ret;
		}
		bool operator == (const LCTable_SP& s0, const LCTable_SP& s1) noexcept {
			return s0->preciseCompare(*s1);
		}
	}
}
