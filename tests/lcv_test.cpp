#include "lcvtest.hpp"
#include "../lvalue.hpp"

namespace rev {
	namespace test {
		// 基本型テスト
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

		// shared_ptr絡みの型テスト
		using Types1 = ::testing::Types<
			void_sp, void_wp,
			std::shared_ptr<int>, std::weak_ptr<int>
		>;
		template <class T>
		using LCV_Test1 = LCV_TestRW<T>;
		TYPED_TEST_CASE(LCV_Test1, Types1);
		TYPED_TEST(LCV_Test1, Push) {
			this->loadSharedPtrModule();
			ASSERT_NO_FATAL_FAILURE(this->pushTest());
		}
		TYPED_TEST(LCV_Test1, Type) {
			this->loadSharedPtrModule();
			ASSERT_NO_FATAL_FAILURE(this->typeTest());
		}

		// tuple, pair絡みの型テスト
		using Types2 = ::testing::Types<
			std::pair<int,const char*>,
			std::tuple<int, const char*, Lua_SP, LCTable_SP>,
			std::tuple<int, std::tuple<void*, lua_CFunction>, lua_State*>,
			std::tuple<>
		>;
		template <class T>
		using LCV_Test2 = LCV_Test<T>;
		TYPED_TEST_CASE(LCV_Test2, Types2);
		TYPED_TEST(LCV_Test2, Push) { ASSERT_NO_FATAL_FAILURE(this->pushTest()); }
		TYPED_TEST(LCV_Test2, Type) { ASSERT_NO_FATAL_FAILURE(this->typeTest()); }
	}
}
