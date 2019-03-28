#include "uniform_ent.hpp"
#include "../gl/program.hpp"

namespace rev {
	UniformEnt::UniformEnt(const GLProgram& p, draw::IQueue& q):
		_program(p),
		_q(q)
	{}
	const GLProgram& UniformEnt::getProgram() const noexcept {
		return _program;
	}
}
