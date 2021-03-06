#include "test.hpp"
#include "../color.hpp"
#include "lubee/src/ieee754.hpp"

namespace rev {
	namespace test {
		struct ColorTest : Random {};
		TEST_F(ColorTest, RGBtoHSVtoRGB) {
			auto& rd = this->mt();
			const auto fnRand = rd.template getUniformF<float>({0.f, 1.f});
			// RGB -> HSV -> RGBと変換して一致しているか
			const RGBColor rgb0(fnRand(), fnRand(), fnRand());
			const HSVColor hsv = rgb0.toHSV();
			const RGBColor rgb1 = hsv.toRGB();

			constexpr auto Th = lubee::ThresholdF<float>(0.5);
			EXPECT_NEAR(rgb0.x, rgb1.x, Th);
			EXPECT_NEAR(rgb0.y, rgb1.y, Th);
			EXPECT_NEAR(rgb0.z, rgb1.z, Th);

			// HSV -> RGBA が HSV -> RGBとAlpha以外一致しているか
			const RGBAColor rgba1 = hsv.toRGBA(1.f);

			// ビット列までピッタリ同じ筈
			EXPECT_EQ(rgb1.x, rgba1.x);
			EXPECT_EQ(rgb1.y, rgba1.y);
			EXPECT_EQ(rgb1.z, rgba1.z);
			EXPECT_EQ(rgba1.w, 1.f);
		}
		TEST_F(ColorTest, HSVtoRGBtoHSV) {
			auto& rd = this->mt();
			const auto fnRand = rd.template getUniformF<float>({0.f, 1.f});
			// HSV -> RGB -> HSVと変換して一致しているか
			const HSVColor hsv0(fnRand(), fnRand(), fnRand());
			const RGBColor rgb = hsv0.toRGB();
			const HSVColor hsv1 = rgb.toHSV();

			constexpr auto CmpTh = lubee::ThresholdF<float>(0.85);
			constexpr auto Th = lubee::ThresholdF<float>(0.5);
			// (明度、彩度がほぼ0の場合は色相が復元できない為)
			if(hsv0.z > CmpTh) {
				if(hsv0.y > CmpTh) {
					EXPECT_NEAR(hsv0.x, hsv1.x, Th);
				}
				EXPECT_NEAR(hsv0.y, hsv1.y, Th);
			}
			EXPECT_NEAR(hsv0.z, hsv1.z, Th);

			// RGB -> HSVA が RGB -> HSVとAlpha以外一致しているか
			const HSVAColor hsva1 = rgb.toHSVA(1.f);

			// ビット列までピッタリ同じ筈
			EXPECT_EQ(hsv1.x, hsva1.x);
			EXPECT_EQ(hsv1.y, hsva1.y);
			EXPECT_EQ(hsv1.z, hsva1.z);
			EXPECT_EQ(hsva1.w, 1.f);
		}
	}
}

