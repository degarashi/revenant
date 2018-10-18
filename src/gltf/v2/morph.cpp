#include "gltf/v2/morph.hpp"
#include "lubee/src/error.hpp"

namespace rev::gltf::v2 {
	void Morph::makeMask(const MorphTarget target, MorphCoeffMask& dst, const WeightV& coeff) const {
		auto cLen = coeff.size();
		cLen = std::min<std::size_t>(N_Morph/width, cLen);

		for(auto& d : dst)
			d = 0;
		const auto w = width;
		const auto idx = targetIndex[target];
		for(std::size_t i=0 ; i<cLen ; i++) {
			dst[i*w+idx] = coeff[i];
		}
	}
}
