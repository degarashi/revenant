#pragma once
#include "lubee/src/random.hpp"
#include <gtest/gtest.h>

namespace rev {
	namespace test {
		class Random : public ::testing::Test {
			private:
				lubee::RandomMT	_mt;
			public:
				Random(): _mt(lubee::RandomMT::Make<4>()) {}
				auto& mt() noexcept {
					return _mt;
				}
		};
		#define USING(t) using t = typename TestFixture::t
	}
}
