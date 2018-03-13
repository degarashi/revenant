#pragma once
#include "gl_state.hpp"
#include "gl_if.hpp"

namespace rev {
	//! OpenGLのBoolステート値設定
	class GL_BState : public GLState {
		public:
			using Func = decltype(&IGL::glEnable);
		private:
			GLenum		_flag;
			Func		_func;
		public:
			GL_BState(bool enable, GLenum flag);
			Type getType() const noexcept override;
			void apply() const override;
			std::size_t getHash() const noexcept override;
			bool operator == (const GLState& s) const noexcept override;
			bool operator == (const GL_BState& s) const noexcept;
			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
