#pragma once
#include "framearray.hpp"

namespace rev::dc {
	//! 時刻から該当するフレーム番号を検索
	struct ISeekFrame : FrameArray {
		struct PosP {
			std::size_t	idx;
			float		time;
		};
		virtual float length() const = 0;
		virtual PosP position(float t) const = 0;
		virtual PosP position(std::size_t idx, float t) const = 0;
	};
}
