#include "seekframe.hpp"
#include "lubee/src/error.hpp"
#include "spine/src/optional.hpp"

namespace rev::dc {
	// ----------------- SeekFrame -----------------
	SeekFrame::SeekFrame(const SVec<float>& p):
		pos(p)
	{}
	float SeekFrame::length() const {
		return pos->back();
	}
	std::size_t SeekFrame::numFrame() const {
		return pos->size();
	}
	namespace {
		float Ratio(const float p, const float p0, const float p1) {
			return (p-p0) / (p1-p0);
		}
		template <class Itr>
		std::pair<Itr, float> Position(Itr begin, const Itr end, const float t) {
			D_Assert0(begin != end);
			auto cur = std::lower_bound(begin, end, t, [](const auto p, const auto t){
				return p < t;
			});
			if(*cur > t) {
				if(cur != begin)
					--cur;
			}
			const auto cur1 = cur+1;
			D_Assert0(cur1 != end);
			return std::make_pair(cur, Ratio(t, *cur, *cur1));
		}
		template <class P>
			spi::Optional<SeekFrame::PosP> CheckOutOfRange(const P& pos, const float t) {
				if(pos.size()<2 || t<=pos.front()) {
					return SeekFrame::PosP {
						.idx = 0,
						.time = 0
					};
				}
				if(t >= pos.back()) {
					return SeekFrame::PosP {
						.idx = pos.size()-1,
						.time = 0
					};
				}
				return spi::none;
			}
	}
	SeekFrame::PosP SeekFrame::position(const float t) const {
		if(const auto ret = CheckOutOfRange(*pos, t))
			return *ret;

		auto& ps = *pos;
		const auto p = Position(ps.begin(), ps.end(), t);
		return PosP{
			.idx = static_cast<std::size_t>(p.first - ps.begin()),
			.time = p.second
		};
	}
	SeekFrame::PosP SeekFrame::position(const std::size_t idx, const float t) const {
		auto& ps = *pos;
		if(t < ps[idx]) {
			return SeekFrame::position(t);
		}

		if(const auto ret = CheckOutOfRange(*pos, t))
			return *ret;
		const auto check = [t, &ps](const std::size_t idx) -> spi::Optional<PosP> {
			if(t <= ps[idx+1]) {
				return PosP {
					.idx = idx,
					.time = Ratio(t, ps[idx], ps[idx+1])
				};
			}
			return spi::none;
		};
		// same frame
		if(const auto ret = check(idx))
			return *ret;
		// next frame
		if(const auto ret = check(idx+1))
			return *ret;

		const auto p = Position(ps.begin()+idx+2, ps.end(), t);
		return PosP{
			.idx = static_cast<std::size_t>(p.first - ps.begin()),
			.time = p.second
		};
	}

	// ----------------- SeekFrame_cached -----------------
	SeekFrame_cached::SeekFrame_cached():
		_prevFrame(0)
	{}
	SeekFrame_cached::PosP SeekFrame_cached::position(const float t) const {
		const auto ret = position(_prevFrame, t);
		_prevFrame = ret.idx;
		return ret;
	}
}
