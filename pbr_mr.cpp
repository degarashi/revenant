#include "pbr_mr.hpp"

namespace rev {
	PBRMetallicRoughness::Tex::Tex():
		coordId(0)
	{}
	PBRMetallicRoughness::Tex::Tex(const HTex& t, const uint32_t coord):
		tex(t),
		coordId(coord)
	{}

	PBRMetallicRoughness::PBRMetallicRoughness():
		colorFactor(1,1,1,1),
		mrFactor(1,1),
		normalScale(1),
		occlusionStrength(1),
		emissiveFactor(1,1,1),
		alphaMode(AlphaMode::Opaque),
		alphaCutoff(.5f),
		doubleSided(false)
	{}
	float PBRMetallicRoughness::getAlphaCutoff() const noexcept {
		if(alphaMode == AlphaMode::Mask)
			return alphaCutoff;
		return -1.f;
	}
}
