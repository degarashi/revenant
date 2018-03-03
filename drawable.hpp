#pragma once
#include "drawtag.hpp"
#include "updater.hpp"

namespace rev {
	class DrawableObj : public Object {
		protected:
			// ソート用の描画タグ
			DrawTag		_dtag;
		public:
			DrawableObj(Priority prio=DefaultPriority);
			const DrawTag& getDTag() const noexcept;
			void setDrawPriority(Priority p) noexcept;
	};
	// Object::PriorityはUpdateObjと兼用の場合に使われる
	template <class T>
	using DrawableObjT = ObjectT<T, DrawableObj>;
}
DEF_LUAIMPORT(rev::DrawableObj)
