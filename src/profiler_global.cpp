#include "profiler_global.hpp"
#include "sdl_mutex.hpp"
#include "spinlock.hpp"

namespace rev::prof {
	// ---------------- Interval----------------
	Interval::Interval(const Profiler::IntervalInfo& src) {
		beginTime = src.tmBegin;
		root = src.root;
		for(auto& nh : src.byName) {
			name_hist.emplace_back(nh.first, nh.second);
		}
	}
	Timepoint Interval::getEndTime() const {
		auto node = root;
		if(node->getSibling())
			node = node->getSibling();
		return beginTime + node->hist.tAccum;
	}

	namespace {
		struct Shared {
			spi::Optional<Timepoint>	until;
			IntervalEntV				interval;

			IntervalEnt& getEntry() {
				const auto thisId = tls_threadID.get();
				auto itr = std::find_if(interval.begin(), interval.end(), [thisId](const auto& b){
					return b.threadId == thisId;
				});
				if(itr == interval.end()) {
					interval.emplace_back(IntervalEnt{thisId, tls_threadName.get(), {}});
					itr = interval.end();
					std::advance(itr, -1);
				}
				return *itr;
			}
		};
		SpinLock<Shared>			g_shared;
	}
	void SetCaptureUntil(const Timepoint t) {
		auto lk = g_shared.lock();
		lk->until = t;
		lk->interval.clear();
	}
	bool IsCapturing() {
		auto lk = g_shared.lockC();
		return static_cast<bool>(lk->until);
	}
	void PreserveThreadInfo() {
		const auto now = Clock::now();
		auto lk = g_shared.lock();
		if(lk->until) {
			const auto until = *lk->until;
			auto& iv = lk->getEntry();

			const auto pushInfo = [&iv](){
				iv.interval.emplace_back(spi::profiler.getPrev());
			};
			if(now < until) {
				pushInfo();
			} else {
				if(iv.interval.empty() || iv.interval.back().beginTime < until) {
					pushInfo();
				}
			}
		}
	}
	spi::Optional<IntervalEntV> GetCaptured() {
		auto lk = g_shared.lock();
		if(lk->until) {
			const auto now = Clock::now();
			const auto until = *lk->until;
			if(now >= until) {
				auto& interval = lk->interval;
				if(!interval.empty()) {
					bool valid = true;
					for(auto& i : interval) {
						auto& iv = i.interval;
						if(iv.empty() ||
							iv.back().beginTime < until)
						{
							valid = false;
							break;
						}
					}
					if(valid) {
						lk->until = spi::none;
						return std::move(lk->interval);
					}
				}
			}
		}
		return spi::none;
	}
}
