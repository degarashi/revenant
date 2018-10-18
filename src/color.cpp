#include "lubee/src/compiler_macro.hpp"
#include "color.hpp"

namespace rev {
	// ------------ HSVColor ------------
	RGBColor HSVColor::toRGB() const noexcept {
		RGBColor rgb(z,z,z);
		if(y > 0) {
			float x6 = x * 6.f;
			auto hi = static_cast<int>(std::floor(x6)) % 6;
			auto f = x6 - hi;
			switch(hi) {
				case 0:
					rgb.y *= 1 - y * (1 - f);
					rgb.z *= 1 - y;
					break;
				case 1:
					rgb.x *= 1 - y * f;
					rgb.z *= 1 - y;
					break;
				case 2:
					rgb.x *= 1 - y;
					rgb.z *= 1 - y * (1 - f);
					break;
				case 3:
					rgb.x *= 1 - y;
					rgb.y *= 1 - y * f;
					break;
				case 4:
					rgb.x *= 1 - y * (1 - f);
					rgb.y *= 1 - y;
					break;
				case 5:
					rgb.y *= 1-y;
					rgb.z *= 1-y*f;
					break;
				default:
					__assume(0);
			}
		}
		return rgb;
	}
	RGBAColor HSVColor::toRGBA(const float a) const noexcept {
		return toRGB().asRGBA(a);
	}
	// ------------ RGBColor ------------
	HSVColor RGBColor::toHSV() const noexcept {
		auto mm = std::minmax({x,y,z});
		HSVColor hsv;

		float max_min = mm.second - mm.first;
		if(max_min <= 1e-4f)
			hsv.x = 0;
		else if(mm.second == x)
			hsv.x = std::fmod(((y - z) / max_min + 6.f), 6.f);
		else if(mm.second == y)
			hsv.x = (z - x) / max_min + 2.f;
		else
			hsv.x = (x - y) / max_min + 4.f;

		hsv.x /= 6.f;
		hsv.y = (mm.second <= 1e-4f) ? 0 : (max_min / mm.second);
		hsv.z = mm.second;
		return hsv;
	}
	HSVAColor RGBColor::toHSVA(const float a) const noexcept {
		return HSVAColor(toHSV(), a);
	}
	RGBAColor RGBColor::asRGBA(const float a) const noexcept {
		return RGBAColor(x,y,z,a);
	}
	const frea::Vec3& RGBColor::asVec3() const noexcept {
		return static_cast<const frea::Vec3&>(*this);
	}
	// ------------ RGBAColor ------------
	RGBAColor::RGBAColor(const RGBColor& c, const float a) noexcept:
		frea::Vec4(c.x, c.y, c.z, a)
	{}
	const RGBColor& RGBAColor::asRGB() const noexcept {
		return reinterpret_cast<const RGBColor&>(*this);
	}
	const frea::Vec4& RGBAColor::asVec4() const noexcept {
		return static_cast<const frea::Vec4&>(*this);
	}
	// ------------ HSVAColor ------------
	HSVAColor::HSVAColor(const HSVColor& h, const float a) noexcept:
		frea::Vec4(h.x, h.y, h.z, a)
	{}
	const HSVColor& HSVAColor::asHSV() const noexcept {
		return reinterpret_cast<const HSVColor&>(*this);
	}
	const frea::Vec4& HSVAColor::asVec4() const noexcept {
		return static_cast<const frea::Vec4&>(*this);
	}
}

