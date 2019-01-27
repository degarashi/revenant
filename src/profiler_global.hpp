#pragma once
#include "spine/src/profiler.hpp"
#include "spine/src/optional.hpp"
#include "clock.hpp"
#include <SDL_thread.h>

namespace rev::prof {
	// 毎フレーム、各スレッドで呼ぶ
	// (profiler.checkIntervalSwitch()がtrueを返した直後)
	void PreserveThreadInfo();
	// 情報を表示するスレッドで呼ぶ
	void SetCaptureUntil(Timepoint t);
	bool IsCapturing();

	using Name = spi::prof::Name;
	using History = spi::prof::History;
	using Block = spi::prof::Block;
	using BlockSP = spi::prof::BlockSP;
	using Profiler = spi::prof::Profiler;
	struct Interval {
		using HistV = std::vector<std::pair<Name, History>>;

		Timepoint			beginTime;
		spi::prof::BlockSP		root;
		HistV		name_hist;

		Interval() = default;
		Interval(const Profiler::IntervalInfo&);
		Timepoint getEndTime() const;
	};
	using IntervalV = std::vector<Interval>;
	struct IntervalEnt {
		SDL_threadID	threadId;
		std::string		threadName;
		IntervalV		interval;
	};
	using IntervalEntV = std::vector<IntervalEnt>;
	// 指定した区間の情報が集っていればそれをMoveで返す
	spi::Optional<IntervalEntV> GetCaptured();
}
