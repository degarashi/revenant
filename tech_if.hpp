#pragma once
#include "handle/opengl.hpp"
#include "gl_header.hpp"
#include "debuggui_if.hpp"
#include <vector>
#include <unordered_set>

namespace rev {
	using Name = std::string;
	class UniformEnt;
	struct VSem_AttrId;
	using VSemAttrV = std::vector<VSem_AttrId>;
	using UniIdSet = std::unordered_set<GLint>;
	using GLState_SPV = std::vector<HGLState>;
	struct ITech : IDebugGui {
		virtual const GLState_SPV& getSetting() const = 0;
		virtual const UniIdSet& getNoDefaultValue() const noexcept = 0;
		virtual const VSemAttrV& getVAttr() const noexcept = 0;
		virtual const HProg& getProgram() const noexcept = 0;
		virtual const UniformEnt& getDefaultValue() const = 0;
		virtual const Name& getName() const noexcept = 0;
	};
}
