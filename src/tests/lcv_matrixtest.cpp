#include "lcv_classtest.hpp"

namespace rev {
	namespace test {
		template <class T>
		struct LCV_Matrix : LCV_ClassTest<T> {
		};
		using TypesM = ::testing::Types<
			LCVPair1<Mat2>, LCVPair1<Mat3>, LCVPair1<Mat4>
		>;
		TYPED_TEST_CASE(LCV_Matrix, TypesM);
		TYPED_TEST(LCV_Matrix, Push) {
			this->pushTest();
		}
		TYPED_TEST(LCV_Matrix, Type) {
			this->typeTest();
		}
	}
}
