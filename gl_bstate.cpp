#include "gl_bstate.hpp"
#include "lubee/hash_combine.hpp"
#include "drawcmd/queue_if.hpp"

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
	void GL_BState::dcmd_apply(draw::IQueue& q) const {
		q.add(DCmd_Apply{_enable, _flag});
	}
	bool GL_BState::operator == (const GLState& s) const noexcept {
		return _Compare(*this, s);
	}
	bool GL_BState::operator == (const GL_BState& bs) const noexcept {
		return _enable == bs._enable &&
				_flag == bs._flag;
	}

	void GL_BState::DCmd_Apply::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Apply*>(p);
		(GL.*cs_func[static_cast<std::size_t>(self.enable)])(self.flag);
	}
}
