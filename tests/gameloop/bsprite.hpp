#pragma once
#include "sprite2d.hpp"

class BoundingSprite : public rev::DrawableObjT<BoundingSprite>,
						public Sprite2D
{
	private:
		frea::Vec2		_svec;
		struct St;
	public:
		BoundingSprite(const rev::HTex& hTex, const frea::Vec2& pos, const frea::Vec2& svec);
};
DEF_LUAIMPORT(BoundingSprite)
