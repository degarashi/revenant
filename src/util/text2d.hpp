#pragma once
#include "text.hpp"
#include "beat/src/pose2d.hpp"

namespace rev {
	namespace util {
		// 1行の縦をY=1としたサイズに内部変換
		// H,V {Negative, Positive, Middle}
		//! テキスト描画クラス (for 2D)
		class Text2D : public beat::g2::Pose, public Text {
			private:
				float		_lineHeight,
							_depth;
			public:
				Text2D(float lh);
				void setLineHeight(float lh);
				void setDepth(float d);
				int draw(IEffect& e) const;
		};
	}
}
