#pragma once
#include "gl_framebuffer.hpp"

namespace rev {
	struct DCmd_Fb : GLFBufferCore {
		// Pair::ResH
		struct Pair {
			bool	bTex;
			GLuint	resId,		// 0番は無効
					faceFlag;
		};

		Pair			ent[Att::NumAttachment];
		lubee::SizeI	size;	//!< Colo0バッファのサイズ

		using GLFBufferCore::GLFBufferCore;
		DCmd_Fb() = default;
		static void MakeCommand_SetAttachment(draw::IQueue& q, const HFbC& fb, const Res (&att)[Att::NumAttachment]);
		static void MakeCommand_ApplyOnly(draw::IQueue& q, const GLuint fbId);
		static void Command(const void* p);
	};
}
