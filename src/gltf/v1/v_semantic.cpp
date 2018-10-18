#include "v_semantic.hpp"
#include "../check.hpp"
#include "../value_loader.hpp"
#include <regex>

namespace rev::gltf::v1 {
	namespace {
		const std::regex re_a_sem("^([A-Z]+)(_([0-9]+))?$");
		const std::pair<std::string, VSemEnum> c_v_semantic[] = {
			{"POSITION", VSemEnum::POSITION},
			{"NORMAL", VSemEnum::NORMAL},
			{"TEXCOORD", VSemEnum::TEXCOORD},
			{"COLOR", VSemEnum::COLOR},
			{"JOINT", VSemEnum::JOINT},
			{"WEIGHT", VSemEnum::WEIGHT},
		};
	}
	VSemantic V_Semantic::operator()(const char* s) const {
		return *VSemanticFromString(s);
	}
	spi::Optional<VSemantic> VSemanticFromString(const char* s) {
		std::cmatch m;
		if(std::regex_match(s, m, re_a_sem)) {
			if(const auto desc_op = FindEnum(c_v_semantic, m[1].str(), [](auto& c, auto& s){
				return c.first == s;}))
			{
				const auto& desc = *desc_op;
				VSemantic vsem;
				vsem.sem = desc.second;
				if(m.empty())
					vsem.index = 0;
				else
					vsem.index = std::atoi(m[2].first);
				return vsem;
			}
		}
		return spi::none;
	}
}
