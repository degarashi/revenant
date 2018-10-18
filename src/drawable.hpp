#pragma once
#include "drawtag.hpp"
#include "object.hpp"

namespace rev {
	class DrawableObj : public IObject {
		protected:
			// ソート用の描画タグ
			DrawTag		_dtag;
		public:
			DrawableObj(Priority prio=DefaultPriority);
			const DrawTag& getDTag() const noexcept;
			void setDrawPriority(Priority p) noexcept;
			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
			DEF_DEBUGGUI_SUMMARYSTR
	};
	// Object::PriorityはUpdateObjと兼用の場合に使われる
	template <class T>
	using DrawableObjT = ObjectT<T, DrawableObj>;
}
DEF_LUAIMPORT(rev::DrawableObj)
