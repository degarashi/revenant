#include "../../gl/bstate.hpp"
#include "../../gl/format.hpp"
#include "../entry_field.hpp"
#include "../spacing.hpp"
#include "../print.hpp"

namespace rev {
	const char* GL_BState::getDebugName() const noexcept {
		return "GL_BooleanState";
	}
	bool GL_BState::property(const bool edit) {
		auto f = debug::EntryField(getDebugName(), edit);
		f.entry(GLFormat::QueryEnumString(_flag).c_str(), _enable);
		return f.modified();
	}
}
