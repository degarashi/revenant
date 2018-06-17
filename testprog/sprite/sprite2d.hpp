#pragma once
#include "beat/pose2d.hpp"
#include "../../handle/opengl.hpp"
#include "../../glx_id.hpp"
#include "../../drawable.hpp"
#include "../../singleton_data.hpp"
#include "../../handle/opengl.hpp"

namespace vertex {
	struct sprite {
		frea::Vec3	pos;
		frea::Vec2	uv;

		const static rev::FWVDecl s_vdecl;
	};
}

namespace rev {
	struct DrawTag;
}
//! 表示テスト用のスプライト
class Sprite2D : public beat::g2::Pose {
	private:
		using Tech = rev::SingletonData<rev::ITech, Sprite2D, 1>;
		using Prim = rev::SingletonData<rev::Primitive, Sprite2D, 0>;
		Prim	_primitive;
		Tech	_tech;

		rev::HTex			_hTex;
		lubee::RangeF		_zRange;
		float				_zOffset,
							_alpha;

	public:
		static std::shared_ptr<rev::Primitive> MakeData(lubee::IConst<0>);
		static rev::HTech MakeData(lubee::IConst<1>);

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
		DEF_DEBUGGUI_NAME
};
DEF_LUAIMPORT(Sprite2DObj)
