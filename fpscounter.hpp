#pragma once
#include "clock.hpp"

namespace rev {
	//! FPS計測クラス
	/*! GameLoop内で使用 */
	class FPSCounter {
		private:
			Timepoint	_tmBegin;
			int			_counter,
						_fps;
		public:
			FPSCounter() noexcept;
			void reset() noexcept;
			bool update() noexcept;
			int getFPS() const noexcept;
	};
}
