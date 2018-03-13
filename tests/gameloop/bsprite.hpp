#pragma once
#include "sprite2d.hpp"

class BoundingSprite : public rev::DrawableObjT<BoundingSprite>,
						public Sprite2D,
						public std::enable_shared_from_this<BoundingSprite>
{
	private:
		rev::HDGroup	_dg;
		frea::Vec2		_svec;
		struct St;
	public:
		BoundingSprite(const rev::HDGroup& dg, const rev::HTex& hTex, const frea::Vec2& pos, const frea::Vec2& svec);
		DEF_DEBUGGUI_NAME
};
DEF_LUAIMPORT(BoundingSprite)
