#include "systeminfo.hpp"
#include "gl/if.hpp"

namespace rev {
	SystemInfo::SystemInfo() noexcept:
		_scrSize{0,0},
		_fps(0)
	{}
	void SystemInfo::setInfo(const lubee::SizeF& sz, const int fps) {
		_scrSize = sz;
		_fps = fps;
		GL.glViewport(0, 0, sz.width, sz.height);
	}
	const lubee::SizeF& SystemInfo::getScreenSize() const noexcept {
		return _scrSize;
	}
	int SystemInfo::getFPS() const noexcept {
		return _fps;
	}
}
