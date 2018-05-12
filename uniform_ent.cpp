#include "uniform_ent.hpp"
#include "gl_program.hpp"

namespace rev {
	void UniformEnt::clearValue() {
		CommandVec::clear();
	}
	void UniformEnt::setProgram(const HProg& p) {
		_program = p;
		clearValue();
	}
	const HProg& UniformEnt::getProgram() const noexcept {
		return _program;
	}
}
