#include "../../gl_bstate.hpp"
#include "../../gl_format.hpp"
#include "../entry_field.hpp"
#include "../spacing.hpp"
#include "../print.hpp"

namespace rev {
	const char* GL_BState::getDebugName() const noexcept {
		return "GL_BooleanState";
	}
	bool GL_BState::property(const bool edit) {
		auto f = debug::EntryField(getDebugName(), edit);
		bool enable = (_func == &IGL::glEnable);
		if(f.entry(GLFormat::QueryEnumString(_flag).c_str(), enable))
			*this = GL_BState(enable, _flag);
		return f.modified();
	}
}
