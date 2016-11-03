#pragma once
#include "../gl_framebuffer.hpp"

namespace rev {
	namespace draw {
		// 毎回GLでAttachする
		class FrameBuff : public GLFBufferCore, public TokenT<FrameBuff> {
			private:
				HFb		_hFb;
				struct Visitor;
				struct Pair {
					bool	bTex;
					Res		handle;
					GLuint	idRes,		// 0番は無効
							faceFlag;
				} _ent[Att::NUM_ATTACHMENT];
				const lubee::SizeI	_size;	//!< Colo0バッファのサイズ

			public:
				// from GLFBufferTmp
				FrameBuff(GLuint idFb, const lubee::SizeI& s);
				// from GLFBuffer
				FrameBuff(const HFb& hFb, const Res (&att)[Att::NUM_ATTACHMENT]);

				void exec() override;
		};
	}
}
