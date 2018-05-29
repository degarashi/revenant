#include "v_semantic.hpp"
#include "check.hpp"
#include <regex>

namespace rev::gltf {
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
}
