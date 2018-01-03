#pragma once
#include "glx_id.hpp"
#include "primitive.hpp"

namespace rev {
	struct ITech;
	using Tech_SP = std::shared_ptr<ITech>;
	using Priority = uint32_t;
	struct DrawTag {
		using TexAr = std::array<HTex, 4>;

		Tech_SP		technique;
		Primitive_SP	primitive;
		TexAr		idTex;
		Priority	priority;
		float		zOffset;

		DrawTag();
	};
}
