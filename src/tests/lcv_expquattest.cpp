#include "lcv_classtest.hpp"

namespace rev {
	namespace test {
		template <class T>
		struct LCV_EQuat : LCV_ClassTest<T> {
		};
		using TypesE = ::testing::Types<
			LCVPair1<frea::ExpQuat>, LCVPair2<frea::AExpQuat, frea::ExpQuat>
		>;
		TYPED_TEST_SUITE(LCV_EQuat, TypesE);
		TYPED_TEST(LCV_EQuat, Push) {
			this->pushTest();
		}
		TYPED_TEST(LCV_EQuat, Type) {
			this->typeTest();
		}
		TYPED_TEST(LCV_EQuat, ReadMember) {
			this->readMemberTestXYZW();
		}
		TYPED_TEST(LCV_EQuat, WriteMember) {
			this->writeMemberTestXYZW();
		}
	}
}
