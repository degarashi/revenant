#pragma once
#include "handle.hpp"
#include "gl_header.hpp"
#include <vector>
#include <unordered_set>

namespace rev {
	class UniformMap;
	struct VSemAttr;
	using VSemAttrV = std::vector<VSemAttr>;
	using UniIdSet = std::unordered_set<GLint>;
	struct ITech {
		virtual ~ITech() {}
		virtual void applySetting() const = 0;
		virtual const UniIdSet& getNoDefaultValue() const noexcept = 0;
		virtual const VSemAttrV& getVAttr() const noexcept = 0;
		virtual const HProg& getProgram() const noexcept = 0;
		virtual const UniformMap& getDefaultValue() const noexcept = 0;
		virtual const Name& getName() const noexcept = 0;
	};
	using Tech_SP = std::shared_ptr<ITech>;
}
