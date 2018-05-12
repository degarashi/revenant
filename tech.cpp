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
	const VSemAttrV& Tech::getVAttr() const noexcept {
		return _vattr;
	}
	const HProg& Tech::getProgram() const noexcept {
		return _uniformDefault.getProgram();
	}
	const UniformEnt& Tech::getDefaultValueQ() const noexcept {
		return _uniformDefault;
	}
	const Name& Tech::getName() const noexcept {
		return _name;
	}
	void Tech::_makeSetupCmd() {
		_uniformDefault.getProgram()->dcmd_export(_setupCmd);
		_setupCmd.append(_uniformDefault);
		for(auto& s : _setting)
			s->dcmd_export(_setupCmd);
	}
	void Tech::dcmd_setup(draw::IQueue& q) const {
		q.append(_setupCmd);
	}
}

