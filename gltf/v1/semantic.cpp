#include "enums.hpp"
#include "../check.hpp"
#include <regex>

namespace rev::gltf::v1 {
	namespace {
		const std::regex re_a_sem("^([A-Z]+)(_([0-9]+))?$");
		const V_Semantic c_v_semantic[] = {
			{"POSITION"},
			{"NORMAL"},
			{"TEXCOORD"},
			{"COLOR"},
			{"JOINT"},
			{"WEIGHT"},
		};
	}
	// --------- V_Semantic ---------
	spi::Optional<VSemantic> V_Semantic::FromString(const char* s) {
		std::cmatch m;
		if(std::regex_match(s, m, re_a_sem)) {
			if(const auto desc_op = FindEnum(c_v_semantic, m[1].str().c_str())) {
				const auto& desc = *desc_op;
				VSemantic vsem;
				vsem.sem = static_cast<VSemEnum::e>(&desc - c_v_semantic);
				if(m.empty())
					vsem.index = 0;
				else
					vsem.index = std::atoi(m[2].first);
				return vsem;
			}
		}
		return spi::none;
	}
	bool V_Semantic::operator == (const std::string& s) const noexcept {
		return sem == s;
	}
	bool V_Semantic::operator == (const char* s) const noexcept {
		return sem.compare(0, sem.length(), s) == 0;
	}

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
