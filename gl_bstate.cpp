#include "gl_bstate.hpp"

namespace rev {
	namespace {
		const GL_BState::Func cs_func[] = {
			&IGL::glDisable, &IGL::glEnable
		};
	}
	GL_BState::GL_BState(const bool enable, const GLenum flag):
		_flag(flag),
		_func(cs_func[static_cast<int>(enable)])
	{}
	std::size_t GL_BState::getHash() const noexcept {
		return _flag ^ reinterpret_cast<intptr_t>(&_func);
	}
	void GL_BState::apply() const {
		(GL.*_func)(_flag);
	}
	bool GL_BState::operator == (const GLState& s) const noexcept {
		return _Compare(*this, s);
	}
	bool GL_BState::operator == (const GL_BState& bs) const noexcept {
		return _flag == bs._flag &&
				_func == bs._func;
	}
}
