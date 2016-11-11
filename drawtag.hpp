#pragma once
#include "lubee/wrapper.hpp"
#include "glx_id.hpp"
#include "handle.hpp"
#include <array>

namespace rev {
	using Priority = uint32_t;
	using IdValue = lubee::Wrapper<int>;
	struct DrawTag {
		using TexAr = std::array<HTex, 4>;
		using VBuffAr = std::array<HVb, 4>;

		//! TechPassIdと事前登録のIdを取り持つ
		struct TPId {
			bool			bId16;		//!< trueならtpId, falseならpreId
			union {
				GL16Id		tpId;
				IdValue		preId;
				uint32_t	value;		//!< ソート時に使用
			};

			TPId();
			TPId& operator = (const GL16Id& id);
			TPId& operator = (const IdValue& id);
			bool valid() const;
		};
		TPId		idTechPass;

		VBuffAr		idVBuffer;
		HIb			idIBuffer;
		TexAr		idTex;
		Priority	priority;
		float		zOffset;

		DrawTag();
	};
}
