#pragma once
#include "glx_id.hpp"
#include "handle.hpp"
#include <array>

namespace rev {
	class Tech;
	using Tech_SP = std::shared_ptr<Tech>;
	using Priority = uint32_t;
	struct DrawTag {
		using TexAr = std::array<HTex, 4>;
		using VBuffAr = std::array<HVb, 4>;

		Tech_SP		technique;
		VBuffAr		idVBuffer;
		HIb			idIBuffer;
		TexAr		idTex;
		Priority	priority;
		float		zOffset;

		DrawTag();
	};
}
