#include "lcvtest.hpp"
#include "../lvalue.hpp"
#include "../dir.hpp"

namespace rev {
	namespace test {
		using Types0 = ::testing::Types<
			LuaNil, bool, const char*, std::string,
			lua_Integer, lua_Number, lua_OtherNumber,
			lua_OtherInteger, lua_IntegerU, lua_OtherIntegerU,
			lua_State*, lua_CFunction, LCValue, void*, Lua_SP, LCTable_SP
		>;
		template <class T>
		using LCV_Test0 = LCV_Test<T>;
		TYPED_TEST_CASE(LCV_Test0, Types0);
		TYPED_TEST(LCV_Test0, Push) { ASSERT_NO_FATAL_FAILURE(this->pushTest()); }
		TYPED_TEST(LCV_Test0, Type) { ASSERT_NO_FATAL_FAILURE(this->typeTest()); }

		using Types1 = ::testing::Types<
			void_sp, void_wp,
			std::shared_ptr<int>, std::weak_ptr<int>
		>;
		template <class T>
		using LCV_Test1 = LCV_TestRW<T>;
		TYPED_TEST_CASE(LCV_Test1, Types1);
		TYPED_TEST(LCV_Test1, Push) {
			auto& lsp = this->_lsp;
			lsp->addResourcePath(Dir::GetProgramDir() + u8"/resource/sys_script/?.lua");
			lsp->loadModule("shared_ptr");
			ASSERT_NO_FATAL_FAILURE(this->pushTest());
		}
		TYPED_TEST(LCV_Test1, Type) { ASSERT_NO_FATAL_FAILURE(this->typeTest()); }
	}
}
