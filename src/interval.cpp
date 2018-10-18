#include "interval.hpp"

namespace rev {
	namespace interval {
		Wait::Wait(const int32_t n):
			wait(n)
		{}
		bool Wait::advance() {
			if(wait > 0)
				--wait;
			else if(wait == 0)
				return true;
			return false;
		}

		EveryN::EveryN(const uint32_t n):
			nth(n),
			cur(0)
		{}
		bool EveryN::advance() {
			if(cur >= nth) {
				cur = 0;
				return true;
			}
			++cur;
			return false;
		}
	}
}
