#include "bstate.hpp"
#include "gl_if.hpp"

namespace rev::draw {
	BState::BState(const GLenum id, const bool enable):
		id(id),
		enable(enable)
	{}

	namespace {
		using BFunc = decltype(&IGL::glEnable);
		const BFunc cs_func[] = {
			&IGL::glDisable, &IGL::glEnable
		};
	}
	void BState::exec() {
		(GL.*cs_func[static_cast<std::size_t>(enable)])(id);
	}
}
