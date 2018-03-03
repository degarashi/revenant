#pragma once
#include "object.hpp"
#include "updater_mgr.hpp"
#include "lvalue.hpp"

namespace rev {
	namespace detail {
		struct ObjectT_LuaBase {
			static bool CallRecvMsg(const Lua_SP& ls, const HObj& hObj, LCValue& dst, const GMessageStr& msg, const LCValue& arg);
		};
		extern const bool c_pauseDefault;
	}
	template <class T, class Base=Object>
	class ObjectT_Lua : public ObjectT<T,Base>, public std::enable_shared_from_this<ObjectT_Lua<T,Base>> {
		private:
			using base = ObjectT<T,Base>;
		protected:
			template <class... Ts>
			spi::Optional<LCValue> _callLuaMethod(const std::string& method, Ts&&... ts) {
				if(const auto& sp = rev_mgr_obj.getLua()) {
					sp->push(this->shared_from_this());
					const LValueS lv(sp->getLS());
					return lv.callMethodNRet(method, std::forward<Ts>(ts)...);
				}
				return spi::none;
			}
			//! Lua側を終端ステートへ移行
			void _setNullState() {
				_callLuaMethod(luaNS::SetState, luaNS::Null);
				_callLuaMethod(luaNS::SwitchState);
			}
		public:
			using base::base;
			void onUpdate(const bool bFirst) override {
				// このassert文は別にここで無くとも良い
				static_assert(std::is_base_of<Resource, ObjectT_Lua<T,Base>>{}, "");

				base::onUpdate(false);
				if(bFirst) {
					if(!base::isDead())
						_callLuaMethod(luaNS::RecvMsg, luaNS::OnUpdate);
				}
			}
			bool onPause() override {
				if(auto ret = _callLuaMethod(luaNS::RecvMsg, luaNS::OnPause)) {
					auto& tbl = boost::get<LCTable_SP>(*ret);
					// FSMachineから返答があったらそれを返す
					if(static_cast<bool>((*tbl).at(1)))
						return static_cast<bool>((*tbl).at(2));
				}
				// なければデフォルト値を返す
				return detail::c_pauseDefault;
			}
			void onResume() override {
				_callLuaMethod(luaNS::RecvMsg, luaNS::OnResume);
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
			bool recvMsg(LCValue& dst, const GMessageStr& msg, const LCValue& arg) override {
				if(const auto& sp = rev_mgr_obj.getLua())
					return detail::ObjectT_LuaBase::CallRecvMsg(sp, this->shared_from_this(), dst, msg, arg);
				return false;
			}
	};

	class U_ObjectUpd : public ObjectT_Lua<U_ObjectUpd> {
		private:
			struct St_None;
		public:
			U_ObjectUpd();
	};
}
DEF_LUAIMPORT(rev::U_ObjectUpd)
