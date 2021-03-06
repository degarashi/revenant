#include "../../lua/impl.hpp"
#include "sprite2d.hpp"

DEF_LUAIMPLEMENT_PTR(
	rev::test::Sprite2D, Sprite2D,
	rev::luaNS::DrawableObj,
	NOTHING,
	NOTHING,
	(setOffset<const frea::Vec2&>)
	(setScaling<const frea::Vec2&>)
	(setRotation<const frea::RadF&>)
	(setZOffset)
	(setZRange)
	(setAlpha),
	(const rev::HTex&)(float)
)
DEF_LUAIMPLEMENT_PTR(
	rev::test::Sprite2DObj, Sprite2DObj,
	"Sprite2D",
	NOTHING,
	NOTHING,
	NOTHING,
	(const rev::HTex&)(float)
)

#include "bsprite.hpp"
DEF_LUAIMPLEMENT_PTR(
	rev::test::BoundingSprite, BoundingSprite,
	"Sprite2DObj",
	NOTHING,
	NOTHING,
	NOTHING,
	(const rev::HDGroup&)(const rev::HTex&)(const frea::Vec2&)(const frea::Vec2&)
)
