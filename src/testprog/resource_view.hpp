#pragma once
#include "spine/src/singleton.hpp"

namespace rev {
	class ResourceView {
		private:
			struct {
				bool	gl,
						scene;
			} _show;
		public:
			ResourceView();
			void drawMenu();
			void drawView();
	};
}
