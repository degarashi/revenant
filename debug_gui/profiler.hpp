#pragma once
#include "../profiler_global.hpp"

namespace rev::debug {
	class Profiler {
		public:
			struct Color {
				uint32_t	rect,
							frame,
							select_rect,
							select_frame;
			};
		private:
			float					_widthRatio,
									_height;
			Color					_color;
			const prof::Interval*	_select;
			prof::IntervalEntV		_data;
		public:
			Profiler();
			void draw();
	};
}
