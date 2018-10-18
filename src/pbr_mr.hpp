#pragma once
#include "handle/opengl.hpp"
#include "frea/src/vector.hpp"
#include "spine/src/enum.hpp"
#include "debuggui_if.hpp"
#include "color.hpp"

namespace rev {
	DefineEnum(AlphaMode,
		(Opaque)
		(Mask)
		(Blend)
	);
	struct PBRMetallicRoughness : IDebugGui {
		struct Tex {
			HTex		tex;
			uint32_t	coordId;

			Tex();
			Tex(const HTex& t, uint32_t coord);
		};

		// -- from glTF material --
		Tex				color;
		RGBAColor		colorFactor;
		Tex				mr;
		frea::Vec2		mrFactor;
		Tex				normal;
		float			normalScale;
		Tex				occlusion;
		float			occlusionStrength;
		Tex				emissive;
		RGBColor		emissiveFactor;

		AlphaMode		alphaMode;
		float			alphaCutoff;
		bool			doubleSided;

		float getAlphaCutoff() const noexcept;

		PBRMetallicRoughness();
		DEF_DEBUGGUI_NAME
		DEF_DEBUGGUI_PROP
	};
	using PBR_SP = std::shared_ptr<PBRMetallicRoughness>;
}
