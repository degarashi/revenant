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
	namespace sysunif {
		namespace screen {
			extern const UniformName
									Size;			// "sys_vScreenSize"
		}
	}
	namespace sysunif3d {
		using namespace sysunif;
		namespace matrix {
			extern const UniformName
									Transform,		// "sys_mTrans"
									TransformInv,	// "sys_mTransInv"
									Proj,			// "sys_mProj"
									ProjInv,		// "sys_mProjInv"
									View,			// "sys_mView"
									ViewInv,		// "sys_mViewInv"
									ViewProj,		// "sys_mViewProj"
									ViewProjInv,	// "sys_mViewProjInv"
									World,			// "sys_mWorld"
									WorldInv,		// "sys_mWorldInv"
									EyePos,			// "sys_vEyePos"
									EyeDir;			// "sys_vEyeDir"
		}
	}
}
