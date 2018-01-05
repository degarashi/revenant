#pragma once
#include "handle.hpp"
#include "gl_header.hpp"
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace rev {
	class UniformEnt;
	struct VSemAttr;
	using VSemAttrV = std::vector<VSemAttr>;
	using UniIdSet = std::unordered_set<GLint>;
	struct GLState;
	using GLState_SP = std::shared_ptr<GLState>;
	using GLState_SPV = std::vector<GLState_SP>;
	struct ITech {
		virtual ~ITech() {}
		virtual const GLState_SPV& getSetting() const = 0;
		virtual const UniIdSet& getNoDefaultValue() const noexcept = 0;
		virtual const VSemAttrV& getVAttr() const noexcept = 0;
		virtual const HProg& getProgram() const noexcept = 0;
		virtual const UniformEnt& getDefaultValue() const = 0;
		virtual const Name& getName() const noexcept = 0;
	};
	using Tech_SP = std::shared_ptr<ITech>;
}
