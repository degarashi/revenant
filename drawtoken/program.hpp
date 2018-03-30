#pragma once
#include "uniform.hpp"
#include "../handle/opengl.hpp"

namespace rev {
	namespace draw {
		class Program : public TokenT<Program> {
			private:
				HProg		_hProg;
				GLuint		_idProg;
			public:
				Program(const HProg& hRes);
				void exec() override;
		};
	}
}
