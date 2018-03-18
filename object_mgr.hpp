#pragma once
#include "handle/object.hpp"
#include "spine/resmgr.hpp"
#include "lubee/alignedalloc.hpp"
#include "spine/singleton.hpp"

namespace rev {
	class LuaState;
	using Lua_SP = std::shared_ptr<LuaState>;

	#define rev_mgr_obj (::rev::ObjMgr::ref())
	//! アクティブゲームオブジェクトの管理
	class ObjMgr : public spi::ResMgr<IObject, lubee::AlignedPool<IObject>>, public spi::Singleton<ObjMgr> {
		private:
			using base_t = spi::ResMgr<IObject, lubee::AlignedPool<IObject>>;
			Lua_SP _lua;
		public:
			~ObjMgr();
			void setLua(const Lua_SP& ls);
			const Lua_SP& getLua() const noexcept;
	};
}
