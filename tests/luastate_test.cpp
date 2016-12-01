#include "../lcv.hpp"
#include "../sdl_rw.hpp"
#include "test.hpp"
#include "lubee/random/string.hpp"

namespace rev {
	namespace test {
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
		class LuaState_Test : public Random {
			protected:
				Lua_SP			_lsp;
				using RdI = decltype(std::declval<Random>().mt().template getUniformF<int>());
				RdI				_rdi;
				int				_maxTableEnt;

				LuaState_Test():
					_lsp(LuaState::NewState()),
					_rdi(mt().template getUniformF<int>())
				{}
				// ----------- GenValue -----------
				void _genValue(bool& dst) {
					dst = _rdi({0,1});
				}
				template <
					class Dst,
					ENABLE_IF(
						std::is_integral<Dst>{} ||
						std::is_floating_point<Dst>{}
					)
				>
				void _genValue(Dst& dst) {
					using L = std::numeric_limits<Dst>;
					dst = mt().template getUniform<Dst>({L::lowest()/4, L::max()/4});
				}
				void _genValue(lua_CFunction& dst) {
					// 予め用意したC関数リストから一つを選ぶ
					dst = CFunction::cs_func[_rdi({0, CFunction::N_Func-1})];
				}
				void _genValue(std::string& dst) {
					dst = lubee::random::GenAlphabetString(_rdi, _rdi({1,32}));
				}
				void _genValue(void*& dst) {
					// ランダムなポインタを作成 -> LightUserDataなので参照されない
					dst = reinterpret_cast<void*>(mt().template getUniform<uintptr_t>());
				}
				void _genValue(Lua_SP& dst) {
					dst = _lsp->newThread();
				}
				template <class V>
				V _genValueReturn() {
					V v;
					_genValue(v);
					return v;
				}
				template <class CB>
				void _genLuaValue(const CB& cb) {
					for(;;) {
						switch(_rdi({0, LuaType::_Num-1})) {
							case LuaType::LNone:
							case LuaType::Nil:
							case LuaType::Userdata:
								// やり直し
								continue;
							case LuaType::Number:
								cb(_genValueReturn<lua_Number>());
								return;
							case LuaType::Boolean:
								cb(_genValueReturn<bool>());
								return;
							case LuaType::String:
								cb(_genValueReturn<std::string>());
								return;
							case LuaType::Table:
								cb(_genValueReturn<LCTable_SP>());
								return;
							case LuaType::Function:
								cb(_genValueReturn<lua_CFunction>());
								return;
							case LuaType::Thread:
								cb(_genValueReturn<Lua_SP>());
								return;
							case LuaType::LightUserdata:
								cb(_genValueReturn<void*>());
								return;
						}
					}
				}
				LCValue _genLCValue() {
					LCValue lcv;
					_genLuaValue([&lcv](auto&& val){
						lcv = val;
					});
					return lcv;
				}
				void _genValue(LCTable_SP& dst) {
					dst = std::make_shared<LCTable>();
					// 配列エントリの初期化
					const int maxEnt = _maxTableEnt;
					_maxTableEnt /= 2;
					const int nAr = _rdi({0, maxEnt});
					for(int i=0 ; i<nAr ; i++) {
						_genLuaValue([&dst, i](auto&& val){
							dst->emplace(LCValue(static_cast<lua_Number>(i)), LCValue(val));
						});
					}
					// 連想配列エントリの初期化
					const int nRec = _rdi({0, maxEnt});
					for(int i=0 ; i<nRec ; i++) {
						for(;;) {
							// キーはテーブル以外
							LCValue key;
							do {
								key = _genLCValue();
							} while(key.type() == LuaType::Table);
							// 既に同じキーを持っている場合は生成しなおし
							if(dst->count(key) == 0) {
								dst->emplace(key, _genLCValue());
								break;
							}
						}
					}
				}

				auto _getValue(const int idx, bool*) {
					return _lsp->toBoolean(idx);
				}
				template <class V, ENABLE_IF(std::is_integral<V>{})>
				auto _getValue(const int idx, V*) {
					return _lsp->toInteger(idx);
				}
				template <class V, ENABLE_IF(std::is_floating_point<V>{})>
				auto _getValue(const int idx, V*) {
					return _lsp->toNumber(idx);
				}
				auto _getValue(const int idx, lua_CFunction*) {
					return _lsp->toCFunction(idx);
				}
				auto _getValue(const int idx, std::string*) {
					return _lsp->toString(idx);
				}
				auto _getValue(const int idx, void**) {
					return _lsp->toUserData(idx);
				}
				auto _getValue(const int idx, Lua_SP*) {
					return _lsp->toThread(idx);
				}
				auto _getValue(const int idx, LCTable_SP*) {
					return _lsp->toTable(idx);
				}
		};
		template <class T>
		struct LCV_ToLua { using type = T; };
		template <> struct LCV_ToLua<std::string> { using type = const char*; };
		template <> struct LCV_ToLua<lua_Number> { using type = lua_Number; };
		template <> struct LCV_ToLua<lua_OtherNumber> { using type = lua_Number; };
		template <> struct LCV_ToLua<lua_Integer> { using type = lua_Number; };
		template <> struct LCV_ToLua<lua_IntegerU> { using type = lua_Number; };
		template <> struct LCV_ToLua<lua_OtherInteger> { using type = lua_Number; };
		template <> struct LCV_ToLua<lua_OtherIntegerU> { using type = lua_Number; };

		template <class T>
		struct LCV_FromLua { using type = T; };
		template <> struct LCV_FromLua<const char*> { using type = std::string; };

		// テーブルの比較はポインタではなく参照先にする
		// std::stringで入力した値はconst char*で取得される為、専用関数を使う
		bool operator == (const LCTable_SP& s0, const LCTable_SP& s1) noexcept {
			return s0->preciseCompare(*s1);
		}

		template <class T>
		class LuaState_TypedTest : public LuaState_Test {};
		using Type0 = ::testing::Types<
			bool,
			lua_OtherInteger,
			lua_OtherIntegerU,
			lua_Integer,
			lua_IntegerU,
			lua_OtherNumber,
			lua_Number,
			std::string,
			void*,
			Lua_SP,
			LCTable_SP,
			lua_CFunction
		>;
		TYPED_TEST_CASE(LuaState_TypedTest, Type0);
		TYPED_TEST(LuaState_TypedTest, Value) {
			using value_t = TypeParam;
			auto lsp = this->_lsp;
			// ランダムな値を生成し、
			this->_maxTableEnt = 16;
			auto v0 = this->template _genValueReturn<value_t>();
			// LuaへPush
			lsp->push(v0);
			// それを取得し
			using v0_t = decltype(v0);
			v0_t v1 = this->_getValue(-1, (v0_t*)nullptr);
			// 中身が同一かチェック
			ASSERT_TRUE(v0 == v1);

			// (LCValueを介したバージョン)
			const LCValue lcv = lsp->toLCValue(-1);
			using VD_t = typename LCV_ToLua<value_t>::type;
			using VD_Dst = typename LCV_FromLua<VD_t>::type;
			const VD_Dst v2 = v0,
						v3 = boost::get<VD_t>(lcv);
			ASSERT_TRUE(v2 == v3);

			// (toValueを介したバージョン)
			const VD_Dst v4 = lsp->template toValue<VD_Dst>(-1);
			ASSERT_TRUE(v2 == v4);
		}
	}
}
