#include "program.hpp"
#include "../gl_program.hpp"

namespace rev {
	namespace draw {
	// ---------------------- draw::Program ----------------------
		Program::Program(const HProg& hProg):
			_hProg(hProg),
			_idProg(hProg->getProgramId())
		{}
		void Program::exec() {
			GL.glUseProgram(_idProg);
		}
	}
}
