#pragma once
#include "spine/singleton.hpp"

namespace rev {
	#define dg_resview (::rev::debug::ResourceView::ref())
	class ResourceView : public spi::Singleton<ResourceView> {
		private:
			struct {
				bool	camera,
						gl,
						scene;
			} _show;
		public:
			ResourceView();
			void drawMenu();
			void drawView();
	};
}
