#pragma once
#include "beat/pose2d.hpp"
#include "../../handle/opengl.hpp"
#include "../../glx_id.hpp"
#include "../../drawable.hpp"
#include "../../singleton_data.hpp"
#include "../../handle/opengl.hpp"

namespace rev::test {
	namespace vertex {
		struct sprite {
			frea::Vec3	pos;
			frea::Vec2	uv;

			const static FWVDecl s_vdecl;
		};
	}

	namespace rev {
		struct DrawTag;
	}
	//! 表示テスト用のスプライト
	class Sprite2D : public beat::g2::Pose {
		private:
			using Tech = SingletonData<ITech, Sprite2D>;
			using Prim = SingletonData<Primitive, Sprite2D>;
			Prim	_primitive;
			Tech	_tech;

			HTex				_hTex;
			lubee::RangeF		_zRange;
			float				_zOffset,
								_alpha;

		public:
			static HPrim MakeData(Prim*);
			static HTech MakeData(Tech*);

			Sprite2D(const HTex& t, float z);
			void draw(IEffect& e) const;
			void setZOffset(float z);
			void setZRange(const lubee::RangeF& r);
			void setAlpha(float a);
			void outputDrawTag(DrawTag& d) const;
	};
	class Sprite2DObj : public DrawableObjT<Sprite2DObj>,
						public Sprite2D
	{
		private:
			struct St;
		public:
			Sprite2DObj(const HTex& t, float d);
			DEF_DEBUGGUI_NAME
	};
}
DEF_LUAIMPORT(rev::test::Sprite2D)
DEF_LUAIMPORT(rev::test::Sprite2DObj)
