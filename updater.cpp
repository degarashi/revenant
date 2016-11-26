#include "updater.hpp"

namespace rev {
	namespace detail {
		extern const bool c_pauseDefault;
	}
	namespace {
		const std::string cs_objname("Object");
	}
	// -------------------- Object --------------------
	Object::Object():
		_bDestroy(false)
	{}
	Priority Object::getPriority() const {
		return 0;
	}
	bool Object::isDead() const {
		return _bDestroy;
	}
	bool Object::hasLuaState() const {
		return false;
	}
	bool Object::onUpdateBase() {
		if(isDead())
			return true;
		onUpdate(true);
		return isDead();
	}
	void Object::onConnected(const HGroup& /*hGroup*/) {}
	void Object::onDisconnected(const HGroup& /*hGroup*/) {}
	void Object::onUpdate(bool /*bFirst*/) {}
	void Object::destroy() {
		_bDestroy = true;
	}
	const char* Object::getResourceName() const noexcept {
		return cs_objname.c_str();
	}
	void Object::enumGroup(const CBFindGroup& /*cb*/, GroupTypeId /*id*/, int /*depth*/) const {
		ExpectF("not supported operation");
	}
	LCValue Object::recvMsg(const GMessageStr& /*id*/, const LCValue& /*arg*/) {
		return LCValue();
	}
	void Object::proc(const CBUpdProc& /*p*/, bool /*bRecursive*/, Priority /*prioBegin*/, Priority /*prioEnd*/) {
		ExpectF("not supported operation");
	}
	void Object::onDraw(IEffect& /*e*/) const {}
	void Object::onDown(ObjTypeId_OP /*prevId*/, const LCValue& /*arg*/) {}
	bool Object::onPause() {
		return detail::c_pauseDefault;
	}
	void Object::onEffectReset() {}
	void Object::onStop() {}
	void Object::onResume() {}
	void Object::onReStart() {}

	// -------------------- U_Object --------------------
	bool U_Object::isNode() const noexcept {
		return false;
	}
	ObjTypeId U_Object::getTypeId() const noexcept {
		return IdT::Id;
	}
}
