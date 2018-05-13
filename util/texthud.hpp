#pragma once
#include "text.hpp"
#include "spine/enum.hpp"
#include "frea/matrix.hpp"
#include "lubee/wrapper.hpp"

namespace rev {
	using SName = spi::FlyweightItem<std::string>;
	namespace util {
		//! テキスト描画クラス (for HUD)
		class TextHUD : public Text {
			private:
				DefineEnum(Coord,
					(Window)		//!< ウィンドウ座標系 (左上を原点としたX+ Y-のピクセル単位)
					(Screen)		//!< スクリーン座標系 (中央を原点としたX[-Asp,Asp], Y[-1,1])
				);
				Coord				_coordType;
				frea::Vec2			_offset,
									_scale;
				float				_depth;

				frea::Mat3 _makeMatrix() const;
			public:
				TextHUD();
				const static SName	U_Text;
				//! ウィンドウ座標系で範囲指定
				void setWindowOffset(const frea::Vec2& ofs);
				//! スクリーン座標系で範囲指定
				void setScreenOffset(const frea::Vec2& ofs);
				void setScale(const frea::Vec2& s);
				void setDepth(float d);
				int draw(IEffect& e) const;
		};
	}
}
