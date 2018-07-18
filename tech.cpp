#include "tech.hpp"
#include "gl_state.hpp"
#include "vertex.hpp"
#include "gl_program.hpp"
#include "uniform_ent.hpp"

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
	void Tech::_makeUniform(UniformEnt&) const {}
	void Tech::dcmd_uniform(draw::IQueue& q) const {
		UniformEnt u(*getProgram(), q);
		_makeUniform(u);
	}
	void Tech::dcmd_setup(draw::IQueue& q) const {
		_program->dcmd_export(q);
		dcmd_uniform(q);
		for(auto& s : _setting)
			s->dcmd_export(q);
	}
	void Tech::dcmd_resetState(draw::IQueue& q) const {
		for(auto& s : _setting)
			s->dcmd_reset(q);
	}
}

