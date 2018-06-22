#pragma once
#include "spine/enum.hpp"
#include <array>
#include <vector>

namespace rev::gltf::v2 {
	using WeightV = std::vector<float>;
	DefineEnum(MorphTarget,
		(Position)
		(Normal)
		(Tangent)
	);
	constexpr std::size_t	N_Morph = 8;
	using MorphCoeffMask = std::array<float, N_Morph>;
	struct Morph {
		uint_fast8_t	width;
		int_fast8_t		targetIndex[MorphTarget::_Num];

		void makeMask(MorphTarget target, MorphCoeffMask& dst, const WeightV& weight) const;
	};
}
