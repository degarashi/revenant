#pragma once
#include "framebuffer.hpp"

namespace rev {
	//! 非ハンドル管理で一時的にFramebufferを使いたい時のヘルパークラス (内部用)
	class GLFBufferTmp : public GLFBufferCore {
		private:
			static void _Attach(GLenum flag, GLuint rb);
			const lubee::SizeI	_size;
		public:
			GLFBufferTmp(GLuint idFb, const lubee::SizeI& s);
			void attachRBuffer(Att::e att, GLuint rb);
			void attachTexture(Att::e att, GLuint id, MipLevel level);
			void attachCubeTexture(Att::e att, GLuint id, GLuint face, MipLevel level);

			void dcmd_export(draw::IQueue& q) const;
	};
}
