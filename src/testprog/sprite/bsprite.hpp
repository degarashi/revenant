#pragma once
#include "sprite2d.hpp"

namespace rev::test {
	class BoundingSprite : public DrawableObjT<BoundingSprite>,
							public Sprite2D,
							public std::enable_shared_from_this<BoundingSprite>
	{
		private:
			HDGroup			_dg;
			frea::Vec2		_svec;
			struct St;
		public:
			BoundingSprite(const HDGroup& dg, const HTex& hTex, const frea::Vec2& pos, const frea::Vec2& svec);
			DEF_DEBUGGUI_NAME
	};
}
DEF_LUAIMPORT(rev::test::BoundingSprite)
