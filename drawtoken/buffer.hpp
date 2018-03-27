#pragma once
#include "../gl_buffer.hpp"
#include "../handle/opengl.hpp"

namespace rev::draw {
	class Buffer :
		public GLBufferCore,
		public TokenT<Buffer>
	{
		private:
			// バッファの参照カウントを維持してるだけなのでこれ自体にはアクセスしない
			// GLBufferCoreの情報だけ使う
			HBuff	_hBuff;
		public:
			Buffer(const GLBufferCore& core, const HBuff& hBuff);
			void exec() override;
	};
}
