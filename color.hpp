#pragma once
#include "frea/vector.hpp"

namespace rev {
	struct RGBColor;
	struct RGBAColor;
	//! HSV色表現(円柱モデル)
	/*! H,S,V各要素0.0 - 1.0で表す */
	struct HSVColor : frea::Vec3 {
		using frea::Vec3::Vec3;
		RGBColor toRGB() const noexcept;
		RGBAColor toRGBA(float a) const noexcept;
	};
	struct HSVAColor;
	//! RGB色表現
	/*! R,G,B各要素0.0 - 1.0で表す */
	struct RGBColor : frea::Vec3 {
		using frea::Vec3::Vec3;
		HSVColor toHSV() const noexcept;
		HSVAColor toHSVA(float a) const noexcept;
		RGBAColor asRGBA(float a) const noexcept;
	};
	//! HSVA色表現(円柱モデル)
	/*! H,S,V,A各要素0.0 - 1.0で表す */
	struct HSVAColor : frea::Vec4 {
		using frea::Vec4::Vec4;
		HSVAColor(const HSVColor& h, float a=1.f) noexcept;
		const HSVColor& asHSV() const noexcept;
	};
	//! RGBA色表現
	/*! R,G,B,A各要素0.0 - 1.0で表す */
	struct RGBAColor : frea::Vec4 {
		using frea::Vec4::Vec4;
		RGBAColor(const RGBColor& c, float a=1.f) noexcept;
		const RGBColor& asRGB() const noexcept;
	};
}

