#include "luatest.hpp"
#include "../sdl_rw.hpp"

namespace rev {
	namespace test {
		using LuaState_Test = LuaTest;
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
		namespace {
			const std::string c_testYield(
				"-- 指定した回数だけ途中でvalをインクリメントしながらyieldする\n"
				"-- 最後にvalを返す\n"
				"function YieldTest(val, n)\n"
				"	for i=1,n do\n"
				"		coroutine.yield(val)\n"
				"		val = val+1\n"
				"		end\n"
				"	return val\n"
				"end\n"
			);
		}
		// load(), resume(), newThread(), getLS_SP(), getMainLS_SP(), GetMainLS_SP()
		TEST_F(LuaState_Test, Coroutine) {
			auto rw = std::make_shared<RWMgr>("ORG", "APP");
			auto lsp = this->_lsp;
			lsp->loadLibraries();
			lsp->loadFromSource(mgr_rw.fromConstTemporal(c_testYield.c_str(), c_testYield.length()), nullptr, true);
			struct Entry {
				Lua_SP	ls;
				int		value,
						nLoop,
						nLoopCur;
			};
			using Vec = std::vector<Entry>;
			Vec vec;
			const int nThread = _rdi({1, 32});
			for(int i=0 ; i<nThread ; i++) {
				Entry v;
				v.ls = lsp->newThread();
				v.nLoop = _rdi({0,32});
				v.nLoopCur = 1;
				v.value = _rdi({-1000, 1000});
				v.ls->getGlobal("YieldTest");
				v.ls->pushArgs(v.value, v.nLoop);
				const auto res = v.ls->resume(nullptr, 2);
				ASSERT_NE(res.first, v.nLoop==0);
				ASSERT_EQ(1, res.second);
				ASSERT_EQ(v.ls->toInteger(-1), v.value);

				ASSERT_EQ(lsp, v.ls->getMainLS_SP());
				ASSERT_EQ(lsp, LuaState::GetMainLS_SP(v.ls->getLS()));
				ASSERT_EQ(v.ls, v.ls->getLS_SP());

				if(v.nLoop > 0)
					vec.emplace_back(v);
			}
			while(!vec.empty()) {
				Vec tmp;
				for(auto& v : vec) {
					const auto res = v.ls->resume(nullptr, 0);
					ASSERT_NE(res.first, v.nLoop==v.nLoopCur);
					++v.nLoopCur;
					ASSERT_EQ(1, res.second);
					ASSERT_EQ(v.ls->toInteger(-1), v.nLoopCur+v.value-1);
					if(res.first)
						tmp.emplace_back(v);
				}
				std::swap(vec, tmp);
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
			auto v0 = this->template _genValue<value_t>();
			// LuaへPush
			lsp->push(v0);
			// それを取得し
			using v0_t = decltype(v0);
			v0_t v1 = LuaTest::_GetValue(lsp, -1, (v0_t*)nullptr);
			// 中身が同一かチェック
			ASSERT_TRUE(v0 == v1);

			// (LCValueを介したバージョン)
			const LCValue lcv = lsp->toLCValue(-1);
			using VRet_t = typename LCV<value_t>::value_t;
			using VD_t = StringTypeCnv_t<VRet_t>;
			const VD_t v2 = v0,
						v3 = boost::get<VRet_t>(lcv);
			ASSERT_TRUE(v2 == v3);

			// (toValueを介したバージョン)
			const VD_t v4 = lsp->template toValue<VD_t>(-1);
			ASSERT_TRUE(v2 == v4);
		}
	}
}
