#pragma once
#include "spine/singleton.hpp"
#include "lubee/size.hpp"

namespace rev {
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
