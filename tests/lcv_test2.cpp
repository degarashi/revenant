#include "lcvtest.hpp"
#include "lubee/rect.hpp"

namespace rev {
	namespace test {
		struct NoImplement {};
		template <class T>
		using Pair0 = std::pair<T, NoImplement>;
		template <class T>
		using Pair1 = std::pair<T,T>;
		template <class T0, class T1>
		using Pair2 = std::pair<T0,T1>;

		template <class T>
		void LuaRegisterClass(LuaState& lsc, T*) {
			LuaImport::RegisterClass<T>(lsc);
		}
		void LuaRegisterClass(LuaState&, NoImplement*) {}

		using Types3 = ::testing::Types<
			Pair0<lubee::Range<int>>, Pair0<lubee::Range<float>>, Pair0<lubee::Range<double>>,
			Pair0<lubee::SizeI>, Pair0<lubee::SizeF>,
			Pair0<lubee::RectI>, Pair0<lubee::RectF>,
			Pair1<frea::DegF>, Pair2<frea::DegD, frea::DegF>, Pair1<frea::RadF>, Pair2<frea::RadD, frea::RadF>
		>;
		template <class T>
		struct LCV_Test3 : LCV_TestRW<typename T::first_type> {
			using value_t = typename T::first_type;
			using lua_type = typename T::second_type;
		};
		TYPED_TEST_CASE(LCV_Test3, Types3);
		TYPED_TEST(LCV_Test3, Push) {
			USING(lua_type);
			this->loadSharedPtrModule();
			LuaRegisterClass(*this->_lsp, (lua_type*)nullptr);
			ASSERT_NO_FATAL_FAILURE(this->pushTest());
		}
		TYPED_TEST(LCV_Test3, Type) {
			USING(lua_type);
			this->loadSharedPtrModule();
			LuaRegisterClass(*this->_lsp, (lua_type*)nullptr);
			ASSERT_NO_FATAL_FAILURE(this->typeTest());
		}
	}
}
