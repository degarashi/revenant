#include "drawable.hpp"
#include "drawsort.hpp"

namespace rev {
	DrawableObj::DrawableObj(const Priority prio) {
		_dtag.priority = prio;
	}
	const DrawTag& DrawableObj::getDTag() const noexcept {
		return _dtag;
	}
	void DrawableObj::setDrawPriority(const Priority p) noexcept {
		_dtag.priority = p;
	}
}
