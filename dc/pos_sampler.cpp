#include "pos_sampler.hpp"
#include "lubee/error.hpp"

namespace rev::dc {
	float PosSampler::length() const {
		return pos->back();
	}
	std::size_t PosSampler::numKey() const {
		return pos->size();
	}
	PosSampler::PosP PosSampler::position(const float t) const {
		if(pos->size() < 2) {
			return PosP {
				.idx = 0,
				.time = 0
			};
		}
		if(t <= pos->front()) {
			return PosP {
				.idx = 0,
				.time = 0
			};
		}
		const auto len = pos->size();
		if(t >= pos->back()) {
			return PosP {
				.idx = len-1,
				.time = 0
			};
		}

		auto beg = std::lower_bound(pos->begin(), pos->end(), t, [](const auto p, const auto t){
			return p < t;
		});
		if(*beg > t) {
			if(beg != pos->begin())
				--beg;
		}
		D_Assert0(beg+1 != pos->end());

		const std::size_t idx0 = beg-pos->begin(),
							idx1 = idx0+1;
		const float time =  (idx0 != idx1) ? ((t-(*pos)[idx0]) / ((*pos)[idx1] - (*pos)[idx0])) : 0;
		return PosP{
			.idx = idx0,
			.time = time
		};
	}
}
