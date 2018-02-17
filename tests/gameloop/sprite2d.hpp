#pragma once
#include "beat/pose2d.hpp"
#include "../../handle.hpp"
#include "../../glx_id.hpp"
#include "../../drawable.hpp"
#include "../../vertex.hpp"
#include "../../singleton_data.hpp"

namespace vertex {
	struct sprite {
		frea::Vec3	pos;
		frea::Vec2	uv;
	};
}
namespace vdecl {
	struct sprite {};
}
DefineVDecl(::vdecl::sprite)

namespace rev {
	class GLEffect_2D;
	struct DrawTag;
}
using Primitive_SP = std::shared_ptr<rev::Primitive>;
//! 表示テスト用のスプライト
class Sprite2D : public beat::g2::Pose {
	private:
		using Prim = rev::SingletonData<rev::Primitive, Sprite2D>;
		Prim				_primitive;
		rev::HTex			_hTex;
		lubee::RangeF		_zRange;
		float				_zOffset,
							_alpha;

	public:
		static Primitive_SP MakeData();
		const static rev::Name	T_Sprite2D;
		Sprite2D(const rev::HTex& t, float z);
		void draw(rev::IEffect& e) const;
		void setZOffset(float z);
		void setZRange(const lubee::RangeF& r);
		void setAlpha(float a);
		void outputDrawTag(rev::DrawTag& d) const;
};
DEF_LUAIMPORT(Sprite2D)

class Sprite2DObj : public rev::DrawableObjT<Sprite2DObj>,
					public Sprite2D
{
	private:
		struct St;
	public:
		Sprite2DObj(const rev::HTex& t, float d);
};
DEF_LUAIMPORT(Sprite2DObj)
