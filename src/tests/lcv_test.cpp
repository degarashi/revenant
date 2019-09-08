#include "lcvtest.hpp"
#include "../lua/lvalue.hpp"

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
		struct LCV_Test0 : LCV_Test {
			using value_t = T;
		};
		TYPED_TEST_SUITE(LCV_Test0, Types0);
		TYPED_TEST(LCV_Test0, Push) {
			USING(value_t);
			ASSERT_NO_FATAL_FAILURE(this->template pushTest<value_t>());
		}
		TYPED_TEST(LCV_Test0, Type) {
			USING(value_t);
			ASSERT_NO_FATAL_FAILURE(this->template typeTest<value_t>());
		}

		// shared_ptr絡みの型テスト
		using Types1 = ::testing::Types<
			void_sp, void_wp,
			std::shared_ptr<int>, std::weak_ptr<int>
		>;
		template <class T>
		struct LCV_Test1 : LCV_TestRW {
			using value_t = T;
		};
		TYPED_TEST_SUITE(LCV_Test1, Types1);
		TYPED_TEST(LCV_Test1, Push) {
			USING(value_t);
			this->loadSharedPtrModule();
			ASSERT_NO_FATAL_FAILURE(this->template pushTest<value_t>());
		}
		TYPED_TEST(LCV_Test1, Type) {
			USING(value_t);
			this->loadSharedPtrModule();
			ASSERT_NO_FATAL_FAILURE(this->template typeTest<value_t>());
		}

		// tuple, pair絡みの型テスト
		using Types2 = ::testing::Types<
			std::pair<int,const char*>,
			std::tuple<int, const char*, Lua_SP, LCTable_SP>,
			std::tuple<int, std::tuple<void*, lua_CFunction>, lua_State*>,
			std::tuple<>
		>;
		template <class T>
		using LCV_Test2 = LCV_Test0<T>;
		TYPED_TEST_SUITE(LCV_Test2, Types2);
		TYPED_TEST(LCV_Test2, Push) {
			USING(value_t);
			ASSERT_NO_FATAL_FAILURE(this->template pushTest<value_t>());
		}
		TYPED_TEST(LCV_Test2, Type) {
			USING(value_t);
			ASSERT_NO_FATAL_FAILURE(this->template typeTest<value_t>());
		}
	}
}
