#pragma once
#include <limits>
#include <cstdint>
#include "spine/optional.hpp"
#include "handle.hpp"
#include "debuggui_if.hpp"

namespace rev {
	using Priority = uint32_t;
	constexpr static Priority DefaultPriority = std::numeric_limits<Priority>::max() / 2;

	using GroupTypeId = uint32_t;		//!< Object種別Id
	using ObjTypeId = uint32_t;			//!< Object種別Id
	using ObjTypeId_OP = spi::Optional<ObjTypeId>;
	using GMessageStr = std::string;
	using CBFindGroup = std::function<bool (const HGroup&)>;
	using CBUpdProc = std::function<void (const HObj&)>;

	class LCValue;
	//! ゲームオブジェクト基底インタフェース
	class IObject :
		public Resource,
		public IDebugGui
	{
		private:
			bool _bDestroy;
			friend class UpdGroup;
			// UpdGroupから呼ばれる
			bool _onUpdate();
		public:
			IObject();
			virtual Priority getPriority() const;
			bool isDead() const;

			//! UpdGroupに登録された時に呼ばれる
			virtual void onConnected(const HGroup& hGroup);
			//! UpdGroupから削除される時に呼ばれる
			virtual void onDisconnected(const HGroup& hGroup);
			//! 各Objが実装するアップデート処理
			virtual void onUpdate(bool execLua);

			virtual bool isNode() const noexcept = 0;
			virtual bool hasLuaState() const;
			virtual void destroy();
			//! 一意に割り振られるオブジェクトの識別IDを取得
			virtual ObjTypeId getTypeId() const = 0;
			// ---- Group用メソッド ----
			virtual void enumGroup(const CBFindGroup& cb, GroupTypeId id, int depth) const;
			//! 特定の優先度範囲のオブジェクトを処理
			virtual void proc(const CBUpdProc& p, bool bRecursive,
								Priority prioBegin=std::numeric_limits<Priority>::lowest(),
								Priority prioEnd=std::numeric_limits<Priority>::max());
			// ---- Message用メソッド ----
			virtual bool recvMsg(LCValue& dst, const GMessageStr& msg, const LCValue& arg);
			// ---------- Object/Scene用メソッド ----------
			virtual void onDraw(IEffect& e) const;
			// ---------- Scene用メソッド ----------
			virtual void onDown(ObjTypeId_OP prevId, const LCValue& arg);
			virtual bool onPause();
			virtual void onStop();
			virtual void onResume();
			virtual void onReStart();

			const char* getResourceName() const noexcept override;
			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
#include "luaimport.hpp"
DEF_LUAIMPORT(rev::IObject)
