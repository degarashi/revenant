#include "if.hpp"

namespace rev {
	namespace detail {
		extern const bool c_pauseDefault;
	}
	namespace {
		const std::string cs_objname("Object");
	}
	IObject::IObject():
		_bDestroy(false)
	{}
	Priority IObject::getPriority() const {
		return 0;
	}
	bool IObject::isDead() const {
		return _bDestroy;
	}
	bool IObject::hasLuaState() const {
		return false;
	}
	bool IObject::_onUpdate() {
		if(isDead())
			return true;
		onUpdate(true);
		return isDead();
	}
	void IObject::onConnected(const HGroup& /*hGroup*/) {}
	void IObject::onDisconnected(const HGroup& /*hGroup*/) {}
	void IObject::onUpdate(bool /*execLua*/) {}
	void IObject::destroy() {
		_bDestroy = true;
	}
	const char* IObject::getResourceName() const noexcept {
		return cs_objname.c_str();
	}
	void IObject::enumGroup(const CBFindGroup& /*cb*/, GroupTypeId /*id*/, int /*depth*/) const {
		ExpectF("not supported operation");
	}
	bool IObject::recvMsg(LCValue& /*dst*/, const GMessageStr& /*id*/, const LCValue& /*arg*/) {
		return false;
	}
	void IObject::proc(const CBUpdProc& /*p*/, bool /*bRecursive*/, Priority /*prioBegin*/, Priority /*prioEnd*/) {
		ExpectF("not supported operation");
	}
	void IObject::onDraw(IEffect& /*e*/) const {}
	void IObject::onDown(ObjTypeId_OP /*prevId*/, const LCValue& /*arg*/) {}
	bool IObject::onPause() {
		return detail::c_pauseDefault;
	}
	void IObject::onStop() {}
	void IObject::onResume() {}
	void IObject::onReStart() {}
}
