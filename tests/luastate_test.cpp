#include "../lcv.hpp"
#include "../sdl_rw.hpp"
#include "test.hpp"
#include "lubee/random/string.hpp"

namespace rev {
	namespace test {
		namespace {
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
		}
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
				V _genValue() {
					V v;
					_genValue(v);
					return v;
				}
				template <class CB>
				void _genLuaValue(const LuaType typ, const CB& cb) {
					switch(typ) {
						case LuaType::Nil:
							cb(LuaNil());
							break;
						case LuaType::Number:
							cb(_genValue<lua_Number>());
							break;
						case LuaType::Boolean:
							cb(_genValue<bool>());
							break;
						case LuaType::String:
							cb(_genValue<std::string>());
							break;
						case LuaType::Table:
							cb(_genValue<LCTable_SP>());
							break;
						case LuaType::Function:
							cb(_genValue<lua_CFunction>());
							break;
						case LuaType::Thread:
							cb(_genValue<Lua_SP>());
							break;
						case LuaType::LightUserdata:
							cb(_genValue<void*>());
							break;
						default:
							AssertF("not supported");
					}
				}

				template <int N, class CB>
				void _genLuaValue(const LuaType (&cand)[N], const CB& cb) {
					_genLuaValue(cand[_rdi({0, N-1})], cb);
				}
				template <class T>
				LCValue _genLCValue(const T& t) {
					LCValue lcv;
					_genLuaValue(t, [&lcv](auto&& val){
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
						_genLuaValue(c_luaTypes_nonil, [&dst, i](auto&& val){
							dst->emplace(LCValue(static_cast<lua_Number>(i)), LCValue(val));
						});
					}
					// 連想配列エントリの初期化
					const int nRec = _rdi({0, maxEnt});
					for(int i=0 ; i<nRec ; i++) {
						for(;;) {
							// キーはテーブル以外
							LCValue key = _genLCValue(c_luaTypes_key);
							// 既に同じキーを持っている場合は生成しなおし
							if(dst->count(key) == 0) {
								dst->emplace(key, _genLCValue(c_luaTypes_nonil));
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

		TEST_F(LuaState_Test, RawAccess) {
			auto lsp = this->_lsp;
			lsp->newTable();
			const std::string key = lubee::random::GenAlphabetString(_rdi, _rdi({0,32})),
							key2 = key+"1";
			const LCValue value = _genLCValue(c_luaTypes_nonil);
			lsp->setField(-1, key, value);

			// メタテーブルの用意
			lsp->newTable();
			const int num = _rdi({0, CFunction::N_Func-1});
			lsp->setField(-1, "__index", CFunction::cs_func[num]);
			static lua_Number check = _genValue<lua_Number>();
			lua_CFunction f =
				[](lua_State* ls) -> int{
					check = lua_tonumber(ls, -1);
					return 0;
				};
			lsp->setField(-1, "__newindex", f);
			lsp->setMetatable(-2);

			// 普通にgetすると、キーのあるエントリに対してはその値が返る
			lsp->getField(-1, key);
			ASSERT_TRUE(value.preciseCompare(lsp->toLCValue(-1)));
			lsp->pop(1);
			// そうでないエントリにはnilではなくnumが返る
			lsp->getField(-1, key2);
			ASSERT_EQ(lua_Number(num), lsp->toNumber(-1));
			lsp->pop(1);
			// table[key]に対してsetすればメタメソッドは呼ばれない
			const LCValue lcv = _genLCValue(c_luaTypes_key);
			lsp->setField(-1, key, lcv);
			lsp->getField(-1, key);
			const LCValue lcv2 = lsp->toLCValue(-1);
			ASSERT_TRUE(lcv.preciseCompare(lcv2));
			lsp->pop(1);
			// テーブルにsetするとcheck変数がその値に書き換わる
			{
				const lua_Number num = _genValue<lua_Number>();
				lsp->setField(-1, key2, num);
				ASSERT_EQ(check, num);
			}

			// 存在するエントリへrawGetField
			lsp->rawGetField(-1, key);
			ASSERT_TRUE(lcv.preciseCompare(lsp->toLCValue(-1)));
			lsp->pop(1);
			// 存在しないエントリへrawGetField
			lsp->rawGetField(-1, key2);
			ASSERT_EQ(LuaType::Nil, lsp->type(-1));
			lsp->pop(1);

			// 存在するエントリへrawSetField
			const lua_Number pre_check = check,
							cur = _genValue<lua_Number>();
			lsp->rawSetField(-1, key, cur);
			lsp->rawGetField(-1, key);
			ASSERT_EQ(cur, lsp->toNumber(-1));
			lsp->pop(1);
			// 存在しないエントリへrawSetField
			lsp->rawSetField(-1, key2, cur);
			lsp->getField(-1, key2);
			ASSERT_EQ(cur, lsp->toNumber(-1));
			ASSERT_EQ(pre_check, check);
		}
		// _checkType
		TEST_F(LuaState_Test, CheckType) {
			auto lsp = this->_lsp;
			for(int i=0 ; i<LuaType::_Num ; i++) {
				const auto from = static_cast<LuaType::e>(i);
				if(from == LuaType::Userdata)
					continue;
				const LuaType typ[1] = {from};
				lsp->push(_genLCValue(typ));
				for(int j=0 ; j<LuaType::_Num ; j++) {
					const auto to = static_cast<LuaType::e>(j);
					if(to == LuaType::Userdata)
						continue;
					if(to == from)
						ASSERT_NO_THROW(lsp->checkType(-1, to));
					else
						ASSERT_THROW(lsp->checkType(-1, to), LuaState::EType);
				}
				lsp->pop(1);
			}
		}
		// cnvString
		TEST_F(LuaState_Test, ConvertString) {
			auto lsp = this->_lsp;
			lsp->loadLibraries();
			using Map = std::unordered_map<LCValue, std::string>;
			Map m;
			const int nVal = _rdi({0,64});
			for(int i=0 ; i<nVal ; i++) {
				const LCValue cv = _genLCValue(c_luaTypes_key);
				if(m.count(cv) == 0) {
					lsp->push(cv);
					m.emplace(cv, lsp->cnvString(-1));
					lsp->pop(1);
				}
			}
			// エントリ内にある値は再度cnvStringしても結果が同じ
			for(auto& ent : m) {
				lsp->push(ent.first);
				ASSERT_EQ(ent.second, lsp->cnvString(-1));
				lsp->pop(1);
			}
		}
		TEST_F(LuaState_Test, PrepareTable) {
			auto lsp = this->_lsp;
			const auto name = _genValue<std::string>();
			const auto key = _genValue<std::string>();
			const auto value = _genValue<lua_Number>();
			const auto fnChk = [&lsp, &name, &key, &value, this](auto&& cb){
				const RewindTop rt(lsp->getLS());
				{
					const CheckTop ct(lsp->getLS());
					// スタック[-1]に対象のキーを持つテーブルがなければ新たに作成
					cb();
					// 新しくprepareされたテーブルが載る
					ASSERT_EQ(rt.getBase()+1, lsp->getTop());
					// 新しく作られたテーブルは空
					ASSERT_EQ(LuaType::Table, lsp->type(-1));
					ASSERT_EQ(0, lsp->getLength(-1));
					lsp->setField(-1, key, value);
					// もう一度同じエントリ名でprepare
					lsp->pop(1);
					cb();
					// 既にテーブルがある場合は単にそのテーブルがスタックに載せる
					ASSERT_EQ(rt.getBase()+1, lsp->getTop());
					lsp->getField(-1, key);
					ASSERT_EQ(value, lsp->toNumber(-1));
					lsp->pop(2);
				}
				// テーブル以外の値がある場合は上書きされる
				lsp->setField(-1, name, value);
				cb();
				ASSERT_EQ(rt.getBase()+1, lsp->getTop());
				ASSERT_EQ(LuaType::Table, lsp->type(-1));
				ASSERT_EQ(0, lsp->getLength(-1));
				lsp->setField(-1, key, value);
			};
			// prepareTable
			lsp->newTable();
			fnChk([&lsp, &name](){
				lsp->prepareTable(-1, name);
			});

			lsp->setTop(0);
			lsp->pushGlobal();
			// prepareTableGlobal
			fnChk([&lsp, &name](){
				lsp->prepareTableGlobal(name);
			});
			lsp->getGlobal(name);
			ASSERT_EQ(LuaType::Table, lsp->type(-1));
			lsp->getField(-1, key);
			ASSERT_EQ(value, lsp->toNumber(-1));
		}
		// setGlobal & getGlobal & pushGlobal
		TEST_F(LuaState_Test, Table) {
			auto lsp = this->_lsp;
			const auto key = _genLCValue(c_luaTypes_key),
						value0 = LCValue(_genValue<std::string>());
			lsp->push(value0);
			lsp->setGlobal(key);

			ASSERT_EQ(0, lsp->getTop());
			lsp->getGlobal(key);
			ASSERT_TRUE(value0.preciseCompare(lsp->toLCValue(-1)));
			lsp->setTop(0);
			lsp->pushGlobal();
			lsp->getField(-1, key);
			ASSERT_TRUE(value0.preciseCompare(lsp->toLCValue(-1)));

			lsp->setTop(0);
			// getField & setField
			lsp->newTable(0,0);
			lsp->setField(-1, key, value0);
			lsp->getField(-1, key);
			ASSERT_TRUE(value0.preciseCompare(lsp->toLCValue(-1)));
			lsp->pop(1);

			lsp->push(key);
			lsp->getTable(-2);
			ASSERT_TRUE(value0.preciseCompare(lsp->toLCValue(-1)));
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
			auto v0 = this->template _genValue<value_t>();
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
