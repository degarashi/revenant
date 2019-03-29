#include "../../object/if.hpp"
#include "../entry_field.hpp"

namespace rev {
	const char* IObject::getDebugName() const noexcept {
		return "IObject";
	}
	bool IObject::property(const bool edit) {
		auto f = debug::EntryField("Object", edit);
		f.show("Priority", getPriority());
		f.show("IsDead", isDead());
		f.show("IsNode", isNode());
		f.show("HasLuaState", hasLuaState());
		f.show("TypeId", getTypeId());
		return f.modified();
	}
}
