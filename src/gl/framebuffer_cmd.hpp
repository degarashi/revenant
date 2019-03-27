#pragma once
#include "framebuffer.hpp"

namespace rev {
	struct DCmd_Fb : GLFBufferCore {
		// Pair::ResH
		struct Pair {
			bool			bTex;
			GLuint			resId;		// 0番は無効
			GLenum			faceFlag;
			uint_fast8_t	level;		// Texture時のみ有効
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
