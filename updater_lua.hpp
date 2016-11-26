#pragma once
#include "updater.hpp"
#include "updater_mgr.hpp"
#include "lvalue.hpp"

namespace rev {
	namespace detail {
		struct ObjectT_LuaBase {
			static LCValue CallRecvMsg(const Lua_SP& ls, const HObj& hObj, const GMessageStr& msg, const LCValue& arg);
		};
		extern const bool c_pauseDefault;
	}
	template <class T, class Base=Object>
	class ObjectT_Lua : public ObjectT<T,Base>, public std::enable_shared_from_this<ObjectT_Lua<T,Base>> {
		private:
			using base = ObjectT<T,Base>;
			HObj _hMe;
			HObj _getHandle() {
				static_assert(std::is_base_of<Resource, ObjectT_Lua<T,Base>>{}, "");
				if(!_hMe)
					_hMe = this->shared_from_this();
				return _hMe;
			}
		protected:
			template <class... Ts>
			LCValue _callLuaMethod(const std::string& method, Ts&&... ts) {
				auto sp = rev_mgr_obj.getLua();
				sp->push(_getHandle());
				LValueS lv(sp->getLS());
				return lv.callMethodNRet(method, std::forward<Ts>(ts)...);
			}
			//! Lua側を終端ステートへ移行
			void _setNullState() {
				_callLuaMethod(luaNS::SetState, luaNS::Null);
				_callLuaMethod(luaNS::SwitchState);
			}
		public:
			using base::base;
			void onUpdate(const bool bFirst) override {
				base::onUpdate(false);
				if(bFirst) {
					if(!base::isDead())
						_callLuaMethod(luaNS::RecvMsg, luaNS::OnUpdate);
				}
			}
			bool onPause() override {
				auto ret = _callLuaMethod(luaNS::RecvMsg, luaNS::OnPause);
				auto& tbl = boost::get<LCTable_SP>(ret);
				// FSMachineから返答があったらそれを返す
				if(static_cast<bool>((*tbl).at(1)))
					return static_cast<bool>((*tbl).at(2));
				// なければデフォルト値を返す
				return detail::c_pauseDefault;
			}
			void onResume() override {
				_callLuaMethod(luaNS::RecvMsg, luaNS::OnResume);
			}
			void onEffectReset() override {
				_callLuaMethod(luaNS::RecvMsg, luaNS::OnEffectReset);
			}
			bool hasLuaState() const override {
				return true;
			}
			void destroy() override {
				if(!base::isDead()) {
					if(!base::hasLuaState()) {
						// Lua側ステートをNullにセット
						_setNullState();
					}
					base::destroy();
				}
			}
			LCValue recvMsg(const GMessageStr& msg, const LCValue& arg) override {
				return detail::ObjectT_LuaBase::CallRecvMsg(rev_mgr_obj.getLua(), _getHandle(), msg, arg);
			}
	};

	class U_ObjectUpd : public ObjectT_Lua<U_ObjectUpd> {
		private:
			struct St_None;
		public:
			U_ObjectUpd();
	};
}
