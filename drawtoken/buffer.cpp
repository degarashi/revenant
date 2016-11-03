#include "buffer.hpp"

namespace rev {
	namespace draw {
		// --------------------------- draw::Buffer ---------------------------
		Buffer::Buffer(const GLBufferCore& core, const HBuff& hBuff):
			GLBufferCore(core),
			_hBuff(hBuff)
		{}
		void Buffer::exec() {
			use_begin();
		}
	}
}
