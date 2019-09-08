#include "lcv_classtest.hpp"
#include "lubee/src/rect.hpp"

namespace rev {
	namespace test {
		using TypesL = ::testing::Types<
			LCVPair0<lubee::Range<int>>, LCVPair0<lubee::Range<float>>, LCVPair0<lubee::Range<double>>,
			LCVPair0<lubee::SizeI>, LCVPair0<lubee::SizeF>,
			LCVPair0<lubee::RectI>, LCVPair0<lubee::RectF>,
			LCVPair1<frea::DegF>, LCVPair2<frea::DegD, frea::DegF>, LCVPair1<frea::RadF>, LCVPair2<frea::RadD, frea::RadF>
		>;
		TYPED_TEST_SUITE(LCV_ClassTest, TypesL);
		TYPED_TEST(LCV_ClassTest, Push) {
			this->pushTest();
		}
		TYPED_TEST(LCV_ClassTest, Type) {
			this->typeTest();
		}
	}
}
