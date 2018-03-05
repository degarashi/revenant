#pragma once
#include "drawgroup.hpp"

namespace rev {
	class SceneBase {
		private:
			HGroup		_update;
			HDGroup		_draw;

		public:
			SceneBase(const HGroup& hUpd, const HDGroup& hDraw);
			~SceneBase();
			void setUpdate(const HGroup& hGroup);
			const HGroup& getUpdate() const noexcept;
			void setDraw(const HDGroup& hDGroup);
			const HDGroup& getDraw() const noexcept;
	};
}
