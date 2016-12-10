#include "luatest.hpp"
#include "../lvalue.hpp"
#include <unordered_set>
#include <boost/optional.hpp>
#include "lubee/logical.hpp"

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

		using TypesF = ::testing::Types<
			std::tuple<>,
			std::tuple<bool, lua_Number, const char*, void*>,
			std::tuple<std::string, LCTable_SP>,
			std::tuple<Lua_SP, lua_Integer, bool, LuaNil>
		>;
		template <class T>
		struct LValueS_FTypedTest : LuaTest {
			using tuple_t = T;
			constexpr static std::size_t tuple_size = std::tuple_size<tuple_t>::value;
			template <class CB, std::size_t... Idx>
			void _check(const CB& cb, LValueS&& lvs, std::index_sequence<Idx...>) {
				cb(lvs, reinterpret_cast<void*>(this), genValue<std::tuple_element_t<Idx, tuple_t>>()...);
			}
			template <class CB>
			void _check(const CB& cb, const LCValue& f) {
				_check(cb, LValueS(_lsp->getLS(), f), std::make_index_sequence<tuple_size>());
			}
			template <std::size_t... Idx>
			void pushRandom(LuaState& lsc, std::index_sequence<Idx...>) {
				auto dummy = [](auto...){};
				dummy((lsc.push(genValue<std::tuple_element_t<Idx, tuple_t>>()),0)...);
			}
			void pushRandom(LuaState& lsc) {
				pushRandom(lsc, std::make_index_sequence<tuple_size>());
			}
			template <std::size_t I>
			using te_t = std::tuple_element_t<I, tuple_t>;
			template <std::size_t... Idx>
			static bool CheckArgs(LuaState& lsc, const int ofs, std::index_sequence<Idx...>) {
				return lubee::And_L(
							(
								lsc.type(Idx+ofs) ==
								LCV<te_t<Idx>>()(lsc.toValue<te_t<Idx>>(Idx+ofs))
							)...
						);
			}
			static bool CheckArgs(LuaState& lsc, const int ofs) {
				return CheckArgs(lsc, ofs, std::make_index_sequence<tuple_size>());
			}
			static bool CheckArgsNRet(const LCTable_SP&, lubee::SZConst<tuple_size>) {
				return true;
			}
			template <std::size_t I>
			static bool CheckArgsNRet(const LCTable_SP& tbl, lubee::SZConst<I>) {
				auto itr = tbl->find(lua_Integer(I+1));
				if(itr == tbl->end())
					return false;
				if(itr->second.type() != LCValue(te_t<I>()).type())
					return false;
				return CheckArgsNRet(tbl, lubee::SZConst<I+1>());
			}
			static bool CheckArgsNRet(const LCValue& lc) {
				if(lc.type() != LuaType::Table)
					return false;
				auto tbl = boost::get<LCTable_SP>(lc);
				return CheckArgsNRet(tbl, lubee::SZConst<0>());
			}
		};
		template <class T>
		constexpr std::size_t LValueS_FTypedTest<T>::tuple_size;
		TYPED_TEST_CASE(LValueS_FTypedTest, TypesF);
		namespace {
			template <class Tuple>
			int CFunc(lua_State* ls) {
				using Test = LValueS_FTypedTest<Tuple>;
				constexpr int size = Test::tuple_size;
				[ls](){
					// [self][args...]
					LuaState lsc(ls, false);
					ASSERT_EQ(size+1, lsc.getTop());
					auto* self = reinterpret_cast<Test*>(lsc.toUserData(1));
					// check arguments
					ASSERT_TRUE(Test::CheckArgs(lsc, 2));
					lsc.setTop(0);
					self->pushRandom(lsc);
					ASSERT_EQ(size+0, lsc.getTop());
				}();
				return size;
			}
			template <class Tuple>
			int MFunc(lua_State* ls) {
				using Test = LValueS_FTypedTest<Tuple>;
				constexpr int size = Test::tuple_size;
				[ls](){
					// [table][self][args...]
					LuaState lsc(ls, false);
					ASSERT_EQ(size+2, lsc.getTop());
					auto* self = reinterpret_cast<Test*>(lsc.toUserData(2));
					// check arguments
					ASSERT_TRUE(Test::CheckArgs(lsc, 3));
					ASSERT_EQ(LuaType::Table, lsc.type(1));
					auto tbl = lsc.toValue<LCTable_SP>(1);
					ASSERT_EQ(1, tbl->size());
					ASSERT_EQ(&MFunc<Tuple>, boost::get<lua_CFunction>((tbl->begin())->second));
					lsc.setTop(0);
					self->pushRandom(lsc);
					ASSERT_EQ(size+0, lsc.getTop());
				}();
				return size;
			}
		}

		TYPED_TEST(LValueS_FTypedTest, Call) {
			USING(tuple_t);
			// call(args...)
			ASSERT_NO_FATAL_FAILURE(
				this->_check([](LValueS& lvs, auto&&... args){
					const CheckTop ct(lvs.getLS());
					lvs.call(std::forward<decltype(args)>(args)...);
				}, &CFunc<tuple_t>);
			);
			// callNRet()
			ASSERT_NO_FATAL_FAILURE(
				this->_check([](LValueS& lvs, auto&&... args){
					const CheckTop ct(lvs.getLS());
					const auto ret = lvs.callNRet(std::forward<decltype(args)>(args)...);
					ASSERT_TRUE(TestFixture::CheckArgsNRet(ret));
				}, &CFunc<tuple_t>);
			);
			// operator()(ret, args...)
			ASSERT_NO_FATAL_FAILURE(
				this->_check([](LValueS& lvs, auto&&... args){
					const CheckTop ct(lvs.getLS());
					tuple_t ret;
					lvs(ret, std::forward<decltype(args)>(args)...);
				}, &CFunc<tuple_t>);
			);
			const auto methodName = this->template genValue<const char*>();
			auto tbl = std::make_shared<LCTable>();
			(*tbl)[methodName] = &MFunc<tuple_t>;
			// callMethod()
			ASSERT_NO_FATAL_FAILURE(
				this->_check([methodName](LValueS& lvs, auto&&... args){
					const CheckTop ct(lvs.getLS());
					lvs.callMethod(methodName, std::forward<decltype(args)>(args)...);
				}, tbl);
			);
			// callMethodNRet()
			ASSERT_NO_FATAL_FAILURE(
				this->_check([=](LValueS& lvs, auto&&... args){
					const CheckTop ct(lvs.getLS());
					const auto ret = lvs.callMethodNRet(methodName, std::forward<decltype(args)>(args)...);
					ASSERT_TRUE(TestFixture::CheckArgsNRet(ret));
				}, tbl);
			);
		}
	}
}
