#pragma once
#include "gl_state.hpp"
#include "gl_if.hpp"

namespace rev {
	//! OpenGLのBoolステート値設定
	class GL_BState : public GLState {
		private:
			bool		_enable;
			GLenum		_flag;
			struct DCmd_Apply {
				bool	enable;
				GLenum	flag;
				static void Command(const void* p);
			};
		public:
			GL_BState(bool enable, GLenum flag);
			Type getType() const noexcept override;
			void dcmd_export(draw::IQueue& q) const override;
			std::size_t getHash() const noexcept override;
			bool operator == (const GLState& s) const noexcept override;
			bool operator == (const GL_BState& s) const noexcept;
			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
