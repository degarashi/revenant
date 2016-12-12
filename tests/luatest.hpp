#pragma once
#include "test.hpp"
#include "lubee/random/string.hpp"
#include "../lcv.hpp"

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
		struct CFunction {
			#define NUM_CFUNC 16
			constexpr static int N_Func = NUM_CFUNC;
			#define PP_CFUNC(z,n,data) \
				static int BOOST_PP_CAT(data,n)(lua_State* ls) { \
					lua_pushinteger(ls, n); \
					return 1; \
				}
			static lua_CFunction cs_func[N_Func];
			BOOST_PP_REPEAT(NUM_CFUNC, PP_CFUNC, Func)
			#undef PP_CFUNC
		};
		#define PP_CFUNC(z,n,data) &CFunction::BOOST_PP_CAT(data,n),
		lua_CFunction CFunction::cs_func[N_Func] = {
			BOOST_PP_REPEAT(NUM_CFUNC, PP_CFUNC, Func)
		};
		#undef PP_CFUNC
		#undef NUM_CFUNC

		class LuaTest : public Random {
			protected:
				Lua_SP			_lsp;
				using RdI = decltype(std::declval<Random>().mt().template getUniformF<int>());
				RdI				_rdi;
				int				_maxTableEnt=16;
				//! 生成するconst char*文字列ストア
				using StrV = std::vector<std::string>;
				StrV			_stringVec;
				using LsV = std::vector<Lua_SP>;
				LsV				_lsVec;

				LuaTest():
					_lsp(LuaState::NewState()),
					_rdi(mt().template getUniformF<int>())
				{}
			public:
				lua_State* getLS() {
					return _lsp->getLS();
				}
				const Lua_SP& getLSP() {
					return _lsp;
				}
				// ----------- GenValue -----------
				void genValue(LuaNil&) {}
				void genValue(bool& dst) {
					dst = _rdi({0,1});
				}
				template <
					class Dst,
					ENABLE_IF(
						std::is_integral<Dst>{} ||
						std::is_floating_point<Dst>{}
					)
				>
				void genValue(Dst& dst) {
					using L = std::numeric_limits<Dst>;
					dst = mt().template getUniform<Dst>({L::lowest()/4, L::max()/4});
				}
				void genValue(lua_CFunction& dst) {
					// 予め用意したC関数リストから一つを選ぶ
					dst = CFunction::cs_func[_rdi({0, CFunction::N_Func-1})];
				}
				void genValue(std::string& dst) {
					dst = lubee::random::GenAlphabetString(_rdi, _rdi({0,32}));
				}
				void genValue(void*& dst) {
					// ランダムなポインタを作成 -> LightUserDataなので参照されない
					dst = reinterpret_cast<void*>(mt().template getUniform<uintptr_t>());
				}
				void genValue(Lua_SP& dst) {
					dst = _lsp->newThread();
				}
				void genValue(const char*& dst) {
					_stringVec.resize(_stringVec.size()+1);
					genValue(_stringVec.back());
					dst = _stringVec.back().c_str();
				}
				void genValue(lua_State*& ls) {
					_lsVec.emplace_back(_lsp->newThread());
					ls = _lsVec.back()->getLS();
				}
				void genValue(LCValue& dst) {
					dst = genLCValue(c_luaTypes);
				}

				template <class V>
				V genValue() {
					V v;
					genValue(v);
					return v;
				}
				template <class CB>
				void genLuaValue(const LuaType typ, const CB& cb) {
					switch(typ) {
						case LuaType::Nil:
							cb(LuaNil());
							break;
						case LuaType::Number:
							cb(genValue<lua_Number>());
							break;
						case LuaType::Boolean:
							cb(genValue<bool>());
							break;
						case LuaType::String:
							cb(genValue<std::string>());
							break;
						case LuaType::Table:
							cb(genValue<LCTable_SP>());
							break;
						case LuaType::Function:
							cb(genValue<lua_CFunction>());
							break;
						case LuaType::Thread:
							cb(genValue<Lua_SP>());
							break;
						case LuaType::LightUserdata:
							cb(genValue<void*>());
							break;
						default:
							AssertF("not supported");
					}
				}

				template <int N, class CB>
				void genLuaValue(const LuaType (&cand)[N], const CB& cb) {
					genLuaValue(cand[_rdi({0, N-1})], cb);
				}
				template <class T>
				LCValue genLCValue(const T& t) {
					LCValue lcv;
					genLuaValue(t, [&lcv](auto&& val){
						lcv = val;
					});
					return lcv;
				}
				template <int N>
				LCValue genLCValue(const LuaType (&cand)[N]) {
					return genLCValue(cand[_rdi({0, N-1})]);
				}
				//! スタックへランダムに値をPush
				template <int N>
				int makeRandomValues(const Lua_SP& ls, const LuaType (&cand)[N], const int n) {
					for(int i=0 ; i<n ; i++)
						ls->push(genLCValue(cand));
					return n;
				}
				void genValue(LCTable_SP& dst) {
					dst = std::make_shared<LCTable>();
					// 配列エントリの初期化
					const int maxEnt = _maxTableEnt;
					_maxTableEnt /= 2;
					const int nAr = _rdi({0, maxEnt});
					for(int i=0 ; i<nAr ; i++) {
						genLuaValue(c_luaTypes_nonil, [&dst, i](auto&& val){
							dst->emplace(LCValue(static_cast<lua_Number>(i)), LCValue(val));
						});
					}
					// 連想配列エントリの初期化
					const int nRec = _rdi({0, maxEnt});
					for(int i=0 ; i<nRec ; i++) {
						for(;;) {
							// キーはテーブル以外
							LCValue key = genLCValue(c_luaTypes_key);
							// 既に同じキーを持っている場合は生成しなおし
							if(dst->count(key) == 0) {
								dst->emplace(key, genLCValue(c_luaTypes_nonil));
								break;
							}
						}
					}
				}
				template <class T>
				static auto GetValue(T& p, const int idx, bool*) {
					return p->toBoolean(idx);
				}
				template <class T, class V, ENABLE_IF(std::is_integral<V>{})>
				static auto GetValue(T& p, const int idx, V*) {
					return p->toInteger(idx);
				}
				template <class T, class V, ENABLE_IF(std::is_floating_point<V>{})>
				static auto GetValue(T& p, const int idx, V*) {
					return p->toNumber(idx);
				}
				template <class T>
				static auto GetValue(T& p, const int idx, lua_CFunction*) {
					return p->toCFunction(idx);
				}
				template <class T>
				static auto GetValue(T& p, const int idx, std::string*) {
					return p->toString(idx);
				}
				template <class T>
				static auto GetValue(T& p, const int idx, void**) {
					return p->toUserData(idx);
				}
				template <class T>
				static auto GetValue(T& p, const int idx, Lua_SP*) {
					return p->toThread(idx);
				}
				template <class T>
				static auto GetValue(T& p, const int idx, LCTable_SP*) {
					return p->toTable(idx);
				}
		};
		template <class T>
		class LuaTestT : public LuaTest {
			public:
				using lua_t = T;
				template <class T2=lua_t, ENABLE_IF((std::is_same<T2,LValueS>{}))>
				lua_State* getLSZ(const Lua_SP& lsp = nullptr) const {
					if(lsp)
						return lsp->getLS();
					return _lsp->getLS();
				}
				template <class T2=lua_t, ENABLE_IF((std::is_same<T2,LValueG>{}))>
				Lua_SP getLSZ(const Lua_SP& lsp = nullptr) const {
					if(lsp)
						return lsp;
					return _lsp;
				}
		};

		// テーブルの比較はポインタではなく参照先にする
		// std::stringで入力した値はconst char*で取得される為、専用関数を使う
		bool operator == (const LCTable_SP& s0, const LCTable_SP& s1) noexcept {
			return s0->preciseCompare(*s1);
		}
	}
}
