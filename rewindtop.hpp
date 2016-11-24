#pragma once
#include <lua.hpp>

namespace rev {
	//! デストラクタでLuaスタック位置を元に戻す
	class RewindTop {
		private:
			lua_State	*const	_ls;
			const int			_base;		//!< 初期化された時点でのスタック位置
			bool				_bReset;	//!< trueならスタック位置をbaseへ戻す
		public:
			RewindTop(lua_State* ls) noexcept;
			void setReset(bool r) noexcept;
			int getNStack() const noexcept;
			~RewindTop();
	};
}
