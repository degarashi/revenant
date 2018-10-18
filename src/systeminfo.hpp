#pragma once
#include "spine/src/singleton.hpp"
#include "lubee/src/size.hpp"

namespace rev {
	// メインスレッドからのアクセス専用
	#define mgr_info (::rev::SystemInfo::ref())
	class SystemInfo : public spi::Singleton<SystemInfo> {
		private:
			lubee::SizeF	_scrSize;
			int				_fps;
		public:
			SystemInfo() noexcept;
			void setInfo(const lubee::SizeF& sz, int fps);
			const lubee::SizeF& getScreenSize() const noexcept;
			int getFPS() const noexcept;
	};
}
#include "luaimport.hpp"
DEF_LUAIMPORT(rev::SystemInfo)
