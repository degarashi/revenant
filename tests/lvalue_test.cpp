#include "luatest.hpp"
#include "../lvalue.hpp"
#include <unordered_set>
#include <boost/optional.hpp>

namespace rev {
	namespace test {
		using LValueS_Test = LuaTest;
		using LValueG_Test = LuaTest;
		// type(), toPointer(), LValue(lua_State*), LValue(lua_State*, const LCValue&), LValue(LValue&&)
		TEST_F(LValueS_Test, Value) {
			auto& lsp = this->_lsp;
			auto* ls = lsp->getLS();
			lsp->checkStack(64);
			std::vector<boost::optional<LValueS>> vec;
			std::unordered_set<const void*> ptr;
			const int n = _rdi({1,32});
			for(int i=0 ; i<n ; i++) {
				const auto lc0 = genLCValue(c_luaTypes);
				int array=0;
				if(lc0.type() == LuaType::Table) {
					if(_rdi({0,1})) {
						array = _rdi({1,32});
						auto& tbl = boost::get<LCTable_SP>(lc0);
						tbl->clear();
						for(int i=1 ; i<=array ; i++) {
							(*tbl)[i] = i;
						}
					}
				}
				// 5通りの方法でLValueを生成
				switch(_rdi({0,4})) {
					// LValue(lua_State*)
					case 0:
						lsp->push(lc0);
						vec.emplace_back(ls);
						break;
					// LValue(lua_State*, const LCValue&)
					case 1:
						vec.emplace_back(LValueS(ls, lc0));
						break;
					// LValue(LValue&&)
					case 2:
					{
						LValueS lv(ls, lc0);
						LValueS lv2(lv);
						vec.emplace_back(std::move(lv));
						ASSERT_EQ(lv2, vec.back());
						break;
					}
					// LValue = LValue&&
					case 3:
					{
						LValueS lv(ls, lc0);
						auto lc1 = genLCValue(c_luaTypes);
						vec.emplace_back(LValueS(ls, lc1));
						vec.back() = std::move(lv);
						ASSERT_TRUE(lc0.preciseCompare(vec.back()->toValue<LCValue>()));
						break;
					}
					// LValue = const LValue&
					case 4:
					{
						vec.emplace_back(LValueS(ls, genLCValue(c_luaTypes)));
						vec.emplace_back(LValueS(ls, lc0));
						auto& lv0 = *vec[vec.size()-2];
						auto& lv1 = *vec[vec.size()-1];
						lv0 = lv1;
						ASSERT_TRUE(lv0.toValue<LCValue>().preciseCompare(lv1.toValue<LCValue>()));
						break;
					}
				}
				auto& lv = vec.back();
				// type
				const auto typ = lc0.type();
				ASSERT_EQ(typ, lv->type());
				if(array > 0)
					ASSERT_EQ(array, lv->length());
				// toPointer
				const void* p = lv->toPointer();
				if(typ == LuaType::Userdata ||
					typ == LuaType::Table ||
					typ == LuaType::Thread ||
					typ == LuaType::Function)
				{
					if(typ != LuaType::Function)
						ASSERT_EQ(0, ptr.count(p));
					ptr.emplace(p);
				}
			}
			// 作成した時とは逆順で破棄する
			std::reverse(vec.begin(), vec.end());
		}
		TEST_F(LValueS_Test, Compare_Equal) {
			auto& lsp = this->_lsp;
			lua_State* ls = lsp->getLS();
			const auto lc0 = genLCValue(c_luaTypes),
						lc1 = genLCValue(c_luaTypes);
			LValueS lv0(ls, lc0),
					lv1(ls, lc1);
			// operator ==
			ASSERT_EQ(lc0==lc1, lv0==lv1);
			// operator !=
			ASSERT_NE(lv0==lv1, lv0!=lv1);
			ASSERT_EQ(lc0!=lc1, lv0!=lv1);
		}
		TEST_F(LValueS_Test, Compare_Less) {
			auto& lsp = this->_lsp;
			lua_State* ls = lsp->getLS();
			const auto v0 = genValue<lua_Number>(),
						v1 = genValue<lua_Number>();
			lsp->push(v0);
			LValueS lv0(ls);
			lsp->push(v1);
			LValueS lv1(ls);
			// operator <
			ASSERT_EQ(v0<v1, lv0<lv1);
			if(lv0 < lv1)
				ASSERT_NE(lv0, lv1);
			// operator <=
			ASSERT_EQ(v0<=v1, lv0<=lv1);
			if(lv0 <= lv1) {
				ASSERT_TRUE((lv0==lv1) || (lv0<lv1));
			}
		}

		template <class T>
		struct LValueS_TypedTest : LuaTest {
			using value_t = T;
			static auto _GetLValue(LValueS& lv, bool*) {
				return lv.toBoolean();
			}
			static auto _GetLValue(LValueS& lv, lua_Integer*) {
				return lv.toInteger();
			}
			static auto _GetLValue(LValueS& lv, lua_Number*) {
				return lv.toNumber();
			}
			static auto _GetLValue(LValueS& lv, void**) {
				return lv.toUserData();
			}
			static auto _GetLValue(LValueS& lv, const char**) {
				return lv.toString();
			}
			static auto _GetLValue(LValueS& lv, LCValue*) {
				return lv.toLCValue();
			}
			static auto _GetLValue(LValueS& lv, LCTable_SP*) {
				return lv.toTable();
			}
			static auto _GetLValue(LValueS& lv, Lua_SP*) {
				return lv.toThread();
			}
		};
		using Types = ::testing::Types<bool, lua_Integer, lua_Number, void*, const char*, LCValue, Lua_SP>;
		TYPED_TEST_CASE(LValueS_TypedTest, Types);
		// to(Value)
		TYPED_TEST(LValueS_TypedTest, ToValue) {
			auto& lsp = this->_lsp;
			USING(value_t);
			const value_t val0 = this->template genValue<value_t>();
			LValueS lv(lsp->getLS(), val0);
			const auto ret = this->_GetLValue(lv, (value_t*)nullptr);
			static_assert(std::is_same<value_t, std::decay_t<decltype(ret)>>{}, "something wrong");
			ASSERT_TRUE(LCValue(val0).preciseCompare(LCValue(ret)));
		}
		// prepareValue
		TEST_F(LValueS_Test, PrepareValue) {
			auto& lsp = this->_lsp;
			const auto lc = genLCValue(c_luaTypes);
			LValueS lv(lsp->getLS(), lc);

			auto lsp2 = lsp->newThread();
			lv.prepareValue(lsp2->getLS());
			lsp2->xmove(lsp, 1);
			ASSERT_TRUE(lsp->compare(-2, -1, LuaState::CMP::Equal));
			lsp->pop(1);
		}

		namespace {
			template <class LV>
			struct Chk {
				template <class Self, class MakeKey>
				void operator()(Self& self, const MakeKey& makeKey) const {
					lua_State* ls = self.getLS();
					const LuaType lct[] = {LuaType::Table};
					LCValue lcTbl = self.genLCValue(lct);
					LCTable_SP tbl = boost::get<LCTable_SP>(lcTbl);

					LV lv(ls, lcTbl);
					const auto key = self.genLCValue(c_luaTypes_key);
					const auto key2 = makeKey(ls, key);
					LV lv2{lv[key2]};
					auto itr = tbl->find(key);
					if(itr != tbl->end())
						ASSERT_TRUE(lv2.toLCValue().preciseCompare(itr->second));
					else
						ASSERT_EQ(LuaType::Nil, lv2.type());
				}
			};
		}
		TEST_F(LValueS_Test, TableAccess) {
			// operator[](const LValue) {const}
			Chk<LValueS>()(*this, [](lua_State* ls, const LCValue& key) {
				return LValueS(ls, key);
			});
			Chk<const LValueS>()(*this, [](lua_State* ls, const LCValue& key) {
				return LValueS(ls, key);
			});
			// operator[](const LCValue) {const}
			Chk<LValueS>()(*this, [](lua_State*, const LCValue& key) {
				return key;
			});
			Chk<const LValueS>()(*this, [](lua_State*, const LCValue& key) {
				return key;
			});
		}
		template <class T>
		struct LValueS_PTypedTest : LuaTest {
			using value0_t = std::tuple_element_t<0, T>;
			using value1_t = std::tuple_element_t<1, T>;
		};
		using TypesP = ::testing::Types<
			std::tuple<bool, lua_Integer>,
			std::tuple<lua_Number, const char*>,
			std::tuple<void*, Lua_SP>,
			std::tuple<Lua_SP, bool>,
			std::tuple<const char*, lua_Number>,
			std::tuple<std::string, LCTable_SP>
		>;
		TYPED_TEST_CASE(LValueS_PTypedTest, TypesP);
		// setField(idx,val)
		TYPED_TEST(LValueS_PTypedTest, SetField) {
			USING(value0_t);
			USING(value1_t);
			auto& lsp = this->_lsp;
			const LuaType lct[] = {LuaType::Table};
			LCValue lcTbl = this->genLCValue(lct);
			LValueS lv(lsp->getLS(), lcTbl);
			const auto key = this->template genValue<value0_t>();
			const auto value = this->template genValue<value1_t>();
			lv.setField(key, value);
			LValueS lv1(lsp->getLS(), value);
			LValueS lv2{lv[key]};
			ASSERT_TRUE(lv1.toLCValue().preciseCompare(lv2.toLCValue()));
		}
	}
}
