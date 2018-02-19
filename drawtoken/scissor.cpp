#include "scissor.hpp"
#include "../gl_framebuffer.hpp"
#include "../gl_if.hpp"

namespace rev {
	namespace draw {
		// MEMO: draw::Viewportのコードと重複しているので後でなんとかする
		Scissor::Scissor(const FBRect& rect):
			_rect(rect)
		{}
		void Scissor::exec() {
			const auto r = _rect.resolve([](){
				return GLFBufferCore::GetCurrentFBSize();
			});
			GL.glScissor(r.x0, r.y0, r.width(), r.height());
		}
	}
}
