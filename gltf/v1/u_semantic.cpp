#include "u_semantic.hpp"
#include "../check.hpp"

namespace rev::gltf::v1 {
	namespace {
		const U_Semantic c_u_semantic[USemantic::_Num] = {
			{"LOCAL",						GL_FLOAT_MAT4},
			{"MODEL",						GL_FLOAT_MAT4},
			{"VIEW",						GL_FLOAT_MAT4},
			{"PROJECTION",					GL_FLOAT_MAT4},
			{"MODELVIEW",					GL_FLOAT_MAT4},
			{"MODELVIEWPROJECTION", 		GL_FLOAT_MAT4},
			{"MODELINVERSE",				GL_FLOAT_MAT4},
			{"VIEWINVERSE",					GL_FLOAT_MAT4},
			{"PROJECTIONINVERSE",			GL_FLOAT_MAT4},
			{"MODELVIEWINVERSE",			GL_FLOAT_MAT4},
			{"MODELVIEWPROJECTIONINVERSE",	GL_FLOAT_MAT4},
			{"MODELINVERSETRANSPOSE",		GL_FLOAT_MAT3},
			{"MODELVIEWINVERSETRANSPOSE",	GL_FLOAT_MAT3},
			{"VIEWPORT",					GL_FLOAT_VEC4},
			{"JOINTMATRIX",					GL_FLOAT_MAT4},
		};
	}
	// --------- U_Semantic ---------
	spi::Optional<USemantic> U_Semantic::FromString(const char* s) {
		if(const auto op = FindEnum(c_u_semantic, s)) {
			return static_cast<USemantic::e>(&(*op) - c_u_semantic);
		}
		return spi::none;
	}
	bool U_Semantic::operator == (const char* s) const noexcept {
		return sem == s;
	}
}
