#include "gl_bstate.hpp"
#include "lubee/hash_combine.hpp"
#include "drawtoken/bstate.hpp"

namespace rev {
	namespace {
		using BFunc = decltype(&IGL::glEnable);
		const BFunc cs_func[] = {
			&IGL::glDisable, &IGL::glEnable
		};
	}
	GL_BState::GL_BState(const bool enable, const GLenum flag):
		_enable(enable),
		_flag(flag)
	{}
	GLState::Type GL_BState::getType() const noexcept {
		return Type::Boolean;
	}
	std::size_t GL_BState::getHash() const noexcept {
		return lubee::hash_combine_implicit(_enable, _flag);
	}
	void GL_BState::apply() const {
		(GL.*cs_func[static_cast<std::size_t>(_enable)])(_flag);
	}
	void GL_BState::getDrawToken(draw::TokenDst& dst) const {
		using UT = draw::BState;
		new(dst.allocate_memory(sizeof(UT), draw::CalcTokenOffset<UT>())) UT(_flag, _enable);
	}
	bool GL_BState::operator == (const GLState& s) const noexcept {
		return _Compare(*this, s);
	}
	bool GL_BState::operator == (const GL_BState& bs) const noexcept {
		return _enable == bs._enable &&
				_flag == bs._flag;
	}
}
