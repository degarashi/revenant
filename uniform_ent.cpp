#include "uniform_ent.hpp"
#include "gl_program.hpp"

namespace rev {
	UniformEnt::UniformEnt(const GLProgram& p):
		_program(&p)
	{}
	void UniformEnt::clearValue() {
		CommandVec::clear();
	}
	const GLProgram& UniformEnt::getProgram() const noexcept {
		return *_program;
	}
}
