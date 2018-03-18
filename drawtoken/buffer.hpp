#pragma once
#include "../gl_buffer.hpp"
#include "../handle/opengl.hpp"

namespace rev {
	namespace draw {
		class Buffer : public GLBufferCore, public TokenT<Buffer> {
			private:
				HBuff	_hBuff;
			public:
				Buffer(const GLBufferCore& core, const HBuff& hBuff);
				void exec() override;
		};
	}
}
