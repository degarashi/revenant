#include "luatest.hpp"
#include "../lvalue.hpp"
#include <unordered_set>
#include <boost/optional.hpp>
#include "lubee/logical.hpp"

namespace rev {
	namespace test {
		template <class LV>
		using LValue_Test = LuaTestT<LV>;
		using VTypes = ::testing::Types<
			LValueG,
			LValueS
		>;
		TYPED_TEST_CASE(LValue_Test, VTypes);
		// type(), toPointer(), LValue(lua_State*), LValue(lua_State*, const LCValue&), LValue(LValue&&)
		TYPED_TEST(LValue_Test, Value) {
			using value_t = typename TestFixture::lua_t;
			auto& lsp = this->_lsp;
			auto& rdi = this->_rdi;
			lsp->checkStack(64);
			std::vector<boost::optional<value_t>> vec;
			std::unordered_set<const void*> ptr;
			auto lsz = this->getLSZ();
			const int n = rdi({1,32});
			for(int i=0 ; i<n ; i++) {
				const auto lc0 = this->genLCValue(c_luaTypes);
				int array=0;
				if(lc0.type() == LuaType::Table) {
					if(rdi({0,1})) {
						array = rdi({1,32});
						auto& tbl = boost::get<LCTable_SP>(lc0);
						tbl->clear();
						for(int i=1 ; i<=array ; i++) {
							(*tbl)[i] = i;
						}
					}
				}
				// 5通りの方法でLValueを生成
				switch(rdi({0,4})) {
					// LValue(lua_State*)
					case 0:
						lsp->push(lc0);
						vec.emplace_back(lsz);
						break;
					// LValue(lua_State*, const LCValue&)
					case 1:
						vec.emplace_back(value_t(lsz, lc0));
						break;
					// LValue(LValue&&)
					case 2:
					{
						value_t lv(lsz, lc0);
						value_t lv2(lv);
						vec.emplace_back(std::move(lv));
						ASSERT_EQ(lv2, vec.back());
						break;
					}
					// LValue = LValue&&
					case 3:
					{
						value_t lv(lsz, lc0);
						auto lc1 = this->genLCValue(c_luaTypes);
						vec.emplace_back(value_t(lsz, lc1));
						vec.back() = std::move(lv);
						ASSERT_TRUE(lc0.preciseCompare(vec.back()->template toValue<LCValue>()));
						break;
					}
					// LValue = const LValue&
					case 4:
					{
						vec.emplace_back(value_t(lsz, this->genLCValue(c_luaTypes)));
						vec.emplace_back(value_t(lsz, lc0));
						auto& lv0 = *vec[vec.size()-2];
						auto& lv1 = *vec[vec.size()-1];
						lv0 = lv1;
						ASSERT_TRUE(lv0.template toValue<LCValue>().preciseCompare(lv1.template toValue<LCValue>()));
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
		TYPED_TEST(LValue_Test, Compare_Equal) {
			using value_t = typename TestFixture::lua_t;
			auto ls = this->getLSZ();
			const auto lc0 = this->genLCValue(c_luaTypes),
						lc1 = this->genLCValue(c_luaTypes);
			value_t lv0(ls, lc0),
					lv1(ls, lc1);
			// operator ==
			ASSERT_EQ(lc0==lc1, lv0==lv1);
			// operator !=
			ASSERT_NE(lv0==lv1, lv0!=lv1);
			ASSERT_EQ(lc0!=lc1, lv0!=lv1);
		}
		TYPED_TEST(LValue_Test, Compare_Less) {
			using value_t = typename TestFixture::lua_t;
			auto& lsp = this->_lsp;
			auto ls = this->getLSZ();
			const auto v0 = this->template genValue<lua_Number>(),
						v1 = this->template genValue<lua_Number>();
			lsp->push(v0);
			value_t lv0(ls);
			lsp->push(v1);
			value_t lv1(ls);
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
		struct LValue_TypedTest2 : LuaTestT<std::tuple_element_t<1,T>> {
			using value_t = std::tuple_element_t<0,T>;
			using lvalue_t = std::tuple_element_t<1,T>;
			static auto _GetLValue(lvalue_t& lv, bool*) {
				return lv.toBoolean();
			}
			static auto _GetLValue(lvalue_t& lv, lua_Integer*) {
				return lv.toInteger();
			}
			static auto _GetLValue(lvalue_t& lv, lua_Number*) {
				return lv.toNumber();
			}
			static auto _GetLValue(lvalue_t& lv, void**) {
				return lv.toUserData();
			}
			static auto _GetLValue(lvalue_t& lv, const char**) {
				return lv.toString();
			}
			static auto _GetLValue(lvalue_t& lv, LCValue*) {
				return lv.toLCValue();
			}
			static auto _GetLValue(lvalue_t& lv, LCTable_SP*) {
				return lv.toTable();
			}
			static auto _GetLValue(lvalue_t& lv, Lua_SP*) {
				return lv.toThread();
			}
		};
		template <class... Ts>
		auto ToTestTypes(std::tuple<Ts...>) -> ::testing::Types<Ts...>;
		template <class T>
		using ToTestTypes_t = decltype(ToTestTypes(std::declval<T>()));

		using Types2 = lubee::seq::ExpandTypes_t2<
			std::tuple,
			std::tuple<
				std::tuple<bool, lua_Integer, lua_Number, void*, const char*, LCValue, Lua_SP>,
				std::tuple<LValueG, LValueS>
			>
		>;
		using Types2_t = ToTestTypes_t<Types2>;
		TYPED_TEST_CASE(LValue_TypedTest2, Types2_t);
		// to(Value)
		TYPED_TEST(LValue_TypedTest2, ToValue) {
			USING(value_t);
			USING(lvalue_t);
			const value_t val0 = this->template genValue<value_t>();
			lvalue_t lv(this->getLSZ(), val0);
			const auto ret = this->_GetLValue(lv, (value_t*)nullptr);
			static_assert(std::is_same<value_t, std::decay_t<decltype(ret)>>{}, "something wrong");
			ASSERT_TRUE(LCValue(val0).preciseCompare(LCValue(ret)));
		}
		// prepareValue
		TYPED_TEST(LValue_Test, PrepareValue) {
			using value_t = typename TestFixture::lua_t;
			auto& lsp = this->_lsp;
			const auto lc = this->genLCValue(c_luaTypes);
			value_t lv(this->getLSZ(), lc);

			auto lsp2 = lsp->newThread();
			lv.prepareValue(lsp2->getLS());
			lsp2->xmove(lsp, 1);
			lv.prepareValue(this->getLS());
			ASSERT_TRUE(lsp->compare(-2, -1, LuaState::CMP::Equal));
			lsp->pop(2);
		}

		namespace {
			template <class LV>
			struct Chk {
				template <class Self, class MakeKey>
				void operator()(Self& self, const MakeKey& makeKey) const {
					auto ls = self.getLSZ();
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
		TYPED_TEST(LValue_Test, TableAccess) {
			using value_t = typename TestFixture::lua_t;
			// operator[](const LValue) {const}
			Chk<value_t>()(*this, [](auto&& ls, const LCValue& key) {
				return value_t(ls, key);
			});
			Chk<const value_t>()(*this, [](auto&& ls, const LCValue& key) {
				return value_t(ls, key);
			});
			// operator[](const LCValue) {const}
			Chk<value_t>()(*this, [](auto&&, const LCValue& key) {
				return key;
			});
			Chk<const value_t>()(*this, [](auto&&, const LCValue& key) {
				return key;
			});
		}
		template <class T>
		struct LValue_PTypedTest : LuaTestT<std::tuple_element_t<0,T>> {
			using lvalue_t = std::tuple_element_t<0, T>;
			using values = std::tuple_element_t<1, T>;
			using value0_t = std::tuple_element_t<0, values>;
			using value1_t = std::tuple_element_t<1, values>;
		};
		using TypesPair = std::tuple<
			std::tuple<bool, lua_Integer>,
			std::tuple<lua_Number, const char*>,
			std::tuple<void*, Lua_SP>,
			std::tuple<Lua_SP, bool>,
			std::tuple<const char*, lua_Number>,
			std::tuple<std::string, LCTable_SP>
		>;
		using TypesP = lubee::seq::ExpandTypes_t2<
			std::tuple,
			std::tuple<
				std::tuple<LValueG, LValueS>,
				TypesPair
			>
		>;
		using TypesP_t = ToTestTypes_t<TypesP>;
		TYPED_TEST_CASE(LValue_PTypedTest, TypesP_t);
		// setField(idx,val)
		TYPED_TEST(LValue_PTypedTest, SetField) {
			USING(lvalue_t);
			USING(value0_t);
			USING(value1_t);
			const LuaType lct[] = {LuaType::Table};
			LCValue lcTbl = this->genLCValue(lct);
			lvalue_t lv(this->getLSZ(), lcTbl);
			const auto key = this->template genValue<value0_t>();
			const auto value = this->template genValue<value1_t>();
			lv.setField(key, value);
			lvalue_t lv1(this->getLSZ(), value);
			lvalue_t lv2{lv[key]};
			ASSERT_TRUE(lv1.toLCValue().preciseCompare(lv2.toLCValue()));
		}

		template <class T>
		struct LValue_FTypedTest : LuaTestT<std::tuple_element_t<0,T>> {
			using src_t = T;
			using lvalue_t = std::tuple_element_t<0,T>;
			using tuple_t = std::tuple_element_t<1,T>;
			constexpr static std::size_t tuple_size = std::tuple_size<tuple_t>::value;
			template <class CB, std::size_t... Idx>
			void _check(const CB& cb, lvalue_t&& lvs, std::index_sequence<Idx...>) {
				cb(lvs, reinterpret_cast<void*>(this), this->template genValue<std::tuple_element_t<Idx, tuple_t>>()...);
			}
			template <class CB>
			void _check(const CB& cb, const LCValue& f) {
				_check(cb, lvalue_t(this->getLSZ(), f), std::make_index_sequence<tuple_size>());
			}
			template <std::size_t... Idx>
			void pushRandom(LuaState& lsc, std::index_sequence<Idx...>) {
				auto dummy = [](auto...){};
				dummy((lsc.push(this->template genValue<std::tuple_element_t<Idx, tuple_t>>()),0)...);
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
			bool checkArgsNRet(const LCTable_SP&, lubee::SZConst<tuple_size>) {
				return true;
			}
			template <std::size_t I>
			bool checkArgsNRet(const LCTable_SP& tbl, lubee::SZConst<I>) {
				auto itr = tbl->find(lua_Integer(I+1));
				if(itr == tbl->end())
					return false;
				auto c0 = itr->second.type();
				auto c1 = LCValue(GenValue_t<te_t<I>>()(*this)).type();
				if(c0 != c1)
					return false;
				return checkArgsNRet(tbl, lubee::SZConst<I+1>());
			}
			bool checkArgsNRet(const LCValue& lc) {
				if(lc.type() != LuaType::Table)
					return false;
				auto tbl = boost::get<LCTable_SP>(lc);
				return checkArgsNRet(tbl, lubee::SZConst<0>());
			}
		};
		template <class T>
		constexpr std::size_t LValue_FTypedTest<T>::tuple_size;

		using TypesF0 = std::tuple<
			std::tuple<>,
			std::tuple<bool, lua_Number, const char*, void*>,
			std::tuple<std::string, LCTable_SP>,
			std::tuple<Lua_SP, lua_Integer, bool, LuaNil>
		>;
		using TypesF = lubee::seq::ExpandTypes_t2<
			std::tuple,
			std::tuple<
				std::tuple<LValueG, LValueS>,
				TypesF0
			>
		>;
		using TypesF_t = ToTestTypes_t<TypesF>;
		TYPED_TEST_CASE(LValue_FTypedTest, TypesF_t);
		namespace {
			template <class T>
			int CFunc(lua_State* ls) {
				using Test = LValue_FTypedTest<T>;
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
			template <class T>
			int MFunc(lua_State* ls) {
				using Test = LValue_FTypedTest<T>;
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
					ASSERT_EQ(&MFunc<T>, boost::get<lua_CFunction>((tbl->begin())->second));
					lsc.setTop(0);
					self->pushRandom(lsc);
					ASSERT_EQ(size+0, lsc.getTop());
				}();
				return size;
			}
		}

		TYPED_TEST(LValue_FTypedTest, Call) {
			USING(src_t);
			USING(tuple_t);
			// call(args...)
			ASSERT_NO_FATAL_FAILURE(
				this->_check([](auto& lvs, auto&&... args){
					const CheckTop ct(lvs.getLS());
					lvs.call(std::forward<decltype(args)>(args)...);
				}, &CFunc<src_t>);
			);
			// callNRet()
			ASSERT_NO_FATAL_FAILURE(
				this->_check([this](auto& lvs, auto&&... args){
					const CheckTop ct(lvs.getLS());
					const auto ret = lvs.callNRet(std::forward<decltype(args)>(args)...);
					ASSERT_TRUE(this->checkArgsNRet(ret));
				}, &CFunc<src_t>);
			);
			// operator()(ret, args...)
			ASSERT_NO_FATAL_FAILURE(
				this->_check([](auto& lvs, auto&&... args){
					const CheckTop ct(lvs.getLS());
					tuple_t ret;
					lvs(ret, std::forward<decltype(args)>(args)...);
				}, &CFunc<src_t>);
			);
			const auto methodName = this->template genValue<const char*>();
			auto tbl = std::make_shared<LCTable>();
			(*tbl)[methodName] = &MFunc<src_t>;
			// callMethod()
			ASSERT_NO_FATAL_FAILURE(
				this->_check([methodName](auto& lvs, auto&&... args){
					const CheckTop ct(lvs.getLS());
					lvs.callMethod(methodName, std::forward<decltype(args)>(args)...);
				}, tbl);
			);
			// callMethodNRet()
			ASSERT_NO_FATAL_FAILURE(
				this->_check([=](auto& lvs, auto&&... args){
					const CheckTop ct(lvs.getLS());
					const auto ret = lvs.callMethodNRet(methodName, std::forward<decltype(args)>(args)...);
					ASSERT_TRUE(this->checkArgsNRet(ret));
				}, tbl);
			);
		}
	}
}
