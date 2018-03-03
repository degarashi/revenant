#pragma once
#include "beat/pose2d.hpp"
#include "../../handle.hpp"
#include "../../glx_id.hpp"
#include "../../drawable.hpp"
#include "../../singleton_data.hpp"

namespace rev {
	class VDecl;
	using VDecl_SP = std::shared_ptr<VDecl>;
}
namespace vertex {
	struct sprite {
		frea::Vec3	pos;
		frea::Vec2	uv;

		const static rev::SingletonDataLazy<rev::VDecl, sprite, 0>	s_vdecl;
		static rev::VDecl_SP MakeData(lubee::IConst<0>);
	};
}

namespace rev {
	class GLEffect_2D;
	struct DrawTag;
}
using Primitive_SP = std::shared_ptr<rev::Primitive>;
//! 表示テスト用のスプライト
class Sprite2D : public beat::g2::Pose {
	private:
		using Prim = rev::SingletonData<rev::Primitive, Sprite2D, 0>;
		Prim				_primitive;
		using DefaultTech = rev::SingletonData<rev::ITech, Sprite2D, 1>;
		static DefaultTech	s_defaultTech;
		rev::HTex			_hTex;
		lubee::RangeF		_zRange;
		float				_zOffset,
							_alpha;

	public:
		static Primitive_SP MakeData(lubee::IConst<0>);
		static rev::Tech_SP MakeData(lubee::IConst<1>);
		static rev::Tech_SP GetDefaultTech();
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
