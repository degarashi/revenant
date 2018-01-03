#include "tech.hpp"
#include "gl_state.hpp"
#include "vertex.hpp"

namespace rev {
	void Tech::applySetting() const {
		for(auto& st : _setting)
			st->apply();
	}
	const UniIdSet& Tech::getNoDefaultValue() const noexcept {
		return _noDefValue;
	}
	const VSemAttrV& Tech::getVAttr() const noexcept {
		return _vattr;
	}
	const HProg& Tech::getProgram() const noexcept {
		return _prog_unif.getProgram();
	}
	const UniformMap& Tech::getDefaultValue() const noexcept {
		return _prog_unif.getUniformValue();
	}
	const Name& Tech::getName() const noexcept {
		return _name;
	}
}

