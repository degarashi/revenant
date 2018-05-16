#include "tech.hpp"
#include "gl_state.hpp"
#include "vertex.hpp"
#include "gl_program.hpp"

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
	void Tech::_makeCmd() {
		_program->dcmd_export(_cmd.setup);
		_cmd.setup.append(_cmd.uniform);
		for(auto& s : _setting)
			s->dcmd_export(_cmd.setup);

		for(auto& s : _setting)
			s->dcmd_reset(_cmd.reset);
	}
	void Tech::dcmd_uniform(draw::IQueue& q) const {
		q.append(_cmd.uniform);
	}
	void Tech::dcmd_setup(draw::IQueue& q) const {
		q.append(_cmd.setup);
	}
	void Tech::dcmd_resetState(draw::IQueue& q) const {
		q.append(_cmd.reset);
	}
}

