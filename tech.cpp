#include "tech.hpp"
#include "gl_state.hpp"
#include "vertex.hpp"

namespace rev {
	const GLState_SPV& Tech::getSetting() const {
		return _setting;
	}
	const UniIdSet& Tech::getNoDefaultValue() const noexcept {
		return _noDefValue;
	}
	const FWVMap& Tech::getVMap() const noexcept {
		return _vmap;
	}
	const HProg& Tech::getProgram() const noexcept {
		return _program;
	}
	const Name& Tech::getName() const noexcept {
		return _name;
	}
	void Tech::_makeSetupCmd() {
		_setupCmd.clear();
		_program->dcmd_export(_setupCmd);
		_setupCmd.append(_uniformCmd);
		for(auto& s : _setting)
			s->dcmd_export(_setupCmd);
	}
	void Tech::dcmd_setup(draw::IQueue& q) const {
		q.append(_setupCmd);
	}
}

