#pragma once
#include "clock.hpp"

namespace rev {
	//! FPS計測クラス
	/*! GameLoop内で使用 */
	class FPSCounter {
		private:
			// microseconds
			uint64_t	_remain;
			int			_counter,
						_fps;
		public:
			FPSCounter() noexcept;
			void reset() noexcept;
			bool update(bool drawn, Duration delta) noexcept;
			int getFPS() const noexcept;
	};
}
