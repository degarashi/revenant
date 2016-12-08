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
				const auto lc0 = _genLCValue(c_luaTypes);
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
						auto lc1 = _genLCValue(c_luaTypes);
						vec.emplace_back(LValueS(ls, lc1));
						vec.back() = std::move(lv);
						ASSERT_TRUE(lc0.preciseCompare(vec.back()->toValue<LCValue>()));
						break;
					}
					// LValue = const LValue&
					case 4:
					{
						vec.emplace_back(LValueS(ls, _genLCValue(c_luaTypes)));
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
			const auto lc0 = _genLCValue(c_luaTypes),
						lc1 = _genLCValue(c_luaTypes);
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
			const auto v0 = _genValue<lua_Number>(),
						v1 = _genValue<lua_Number>();
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
			const value_t val0 = this->template _genValue<value_t>();
			LValueS lv(lsp->getLS(), val0);
			const auto ret = this->_GetLValue(lv, (value_t*)nullptr);
			static_assert(std::is_same<value_t, std::decay_t<decltype(ret)>>{}, "something wrong");
			ASSERT_TRUE(LCValue(val0).preciseCompare(LCValue(ret)));
		}
	}
}
