#pragma once
#include "drawtoken_t.hpp"
#include "../handle.hpp"

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
