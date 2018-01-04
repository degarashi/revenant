#include "tech.hpp"
#include "gl_state.hpp"
#include "vertex.hpp"
#include "uniform.hpp"

namespace rev {
	const GLState_SPV& Tech::getSetting() const {
		return _setting;
	}
	const UniIdSet& Tech::getNoDefaultValue() const noexcept {
		return _noDefValue;
	}
	const VSemAttrV& Tech::getVAttr() const noexcept {
		return _vattr;
	}
	const HProg& Tech::getProgram() const noexcept {
		return _program;
	}
	const UniformEnt& Tech::getDefaultValue() const noexcept {
		return _uniform;
	}
	const Name& Tech::getName() const noexcept {
		return _name;
	}
}

