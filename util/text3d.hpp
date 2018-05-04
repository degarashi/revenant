#pragma once
#include "text.hpp"
#include "beat/pose3d.hpp"

namespace rev {
	namespace util {
		//! テキスト描画クラス (for 3D sprite)
		class Text3D : public beat::g3::Pose, public Text {
			private:
				float	_lineHeight;
				bool	_bBillboard;		//!< trueなら描画時にビルボード変換
			public:
				Text3D(float lh, bool bBillboard);
				void setLineHeight(float lh);
				void setBillboard(bool b);
				int draw(IEffect& e) const;
		};
	}
}
