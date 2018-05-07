#pragma once
#include "lubee/wrapper.hpp"
#include "spine/flyweight_item.hpp"

namespace rev {
	using UniformName = spi::FlyweightItem<std::string>;
	namespace unif {
		extern const UniformName
									Alpha,			// "u_alpha"
									Color;			// "u_color"
		namespace texture {
			extern const UniformName
									Diffuse;		// "u_texDiffuse"
		}
	}
	namespace unif2d {
		using namespace unif;
		extern const UniformName
									Depth;			// "u_depth"
	}
	namespace unif3d {
		using namespace unif;
		namespace texture {
			using namespace unif::texture;
			extern const UniformName
									Specular,		// "u_texSpecular"
									Normal,			// "u_texNormal"
									Emissive;		// "u_texEmissive"
		}
	}
}
