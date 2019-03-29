#pragma once
#include "beat/src/pose3d.hpp"
#include "../../object/drawable.hpp"
#include "../../singleton_data.hpp"

namespace rev::test {
	namespace vertex {
		struct cube {
			frea::Vec3	pos;
			frea::Vec3	normal;

			const static FWVDecl s_vdecl;
		};
	}
	class Cube : public beat::g3::Pose,
				public lubee::AAllocator<Cube>
	{
		private:
			using Vec3 = frea::Vec3;
			using Flat = SingletonData<Primitive, Cube, 0>;
			using Gouraud = SingletonData<Primitive, Cube, 1>;
			using Tech = SingletonData<ITech, Cube, 2>;
			Flat	_p_flat;
			Gouraud	_p_gouraud;
			Tech	_tech;
			bool	_flat;
			HPrim _getPrimitive() const noexcept;

		public:
			static HPrim MakeData(Flat*);
			static HPrim MakeData(Gouraud*);
			static HTech MakeData(Tech*);

			Cube();
			void setFlat(bool flat);
			void draw(IEffect& e) const;
			void outputDrawTag(DrawTag& d) const;
	};
	class CubeObj :
		public DrawableObjT<CubeObj>,
		public Cube,
		public lubee::AAllocator<CubeObj>
	{
		private:
			struct St;
		public:
			using AA = lubee::AAllocator<CubeObj>;
			using AA::NewS;
			CubeObj(const frea::Vec3& scale);

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
