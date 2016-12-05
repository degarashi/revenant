#include "luatest.hpp"
#include "../lvalue.hpp"
#include "lubee/rect.hpp"

namespace rev {
	namespace test {
		template <class T>
		struct LCV_Test : LuaTest {
			using value_t = T;
			using LCV_t = LCV<T>;
			LCV_t	_lcv;

			void pushTest() {
				auto& lsp = this->_lsp;
				lua_State *const ls = lsp->getLS();
				lsp->checkStack(32);
				constexpr int Max_Other = 8;
				LCValue prev[Max_Other],
						post[Max_Other];
				const int nprev = _rdi({0,Max_Other-1}),
						npost = _rdi({0,Max_Other-1});
				// 前に関係ない値を挟む
				for(int i=0 ; i<nprev ; i++) {
					prev[i] = _genLCValue(c_luaTypes_key);
					lsp->push(prev[i]);
				}
				// LCV経由でPush
				const auto v0 = _genValue<value_t>();
				const int cur = _lcv(ls, v0);
				ASSERT_EQ(nprev+cur, lsp->getTop());
				// 後に関係ない値を加える
				for(int i=0 ; i<npost ; i++) {
					post[i] = _genLCValue(c_luaTypes_key);
					lsp->push(post[i]);
				}
				// LCV経由で値を取得(正のインデックス & 負のインデックス)
				const auto v1 = _lcv(nprev+1, ls, nullptr),
							v2 = _lcv(lsp->getTop()-npost, ls, nullptr);
				ASSERT_EQ(LCV_FromLua_t<value_t>(v0), v1);
				ASSERT_EQ(v1, v2);

				// スタックの他の値は変化なし
				for(int i=0 ; i<nprev ; i++)
					ASSERT_TRUE(prev[i].preciseCompare(lsp->toLCValue(i+1)));
				for(int i=0 ; i<npost ; i++)
					ASSERT_TRUE(post[i].preciseCompare(lsp->toLCValue(nprev+cur+1+i)));
			}
			void typeTest() {
				// LCV()が返す型タイプが実際Pushした時と一致しているか確認
				auto& lsp = this->_lsp;
				_lcv(lsp->getLS(), _genValue<value_t>());
				ASSERT_EQ(_lcv(), lsp->type(-1));
			}
		};
		using Types0 = ::testing::Types<
			LuaNil, bool, const char*, std::string,
			lua_Integer, lua_Number, lua_OtherNumber,
			lua_OtherInteger, lua_IntegerU, lua_OtherIntegerU,
			lua_State*
		>;
		template <class T>
		struct LCV_Test0 : LCV_Test<T> {};
		TYPED_TEST_CASE(LCV_Test0, Types0);
		TYPED_TEST(LCV_Test0, Push) { ASSERT_NO_FATAL_FAILURE(this->pushTest()); }
		TYPED_TEST(LCV_Test0, Type) { ASSERT_NO_FATAL_FAILURE(this->typeTest()); }
	}
}
