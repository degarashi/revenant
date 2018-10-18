#pragma once
#include "../../handle/opengl.hpp"
#include "../../singleton_data.hpp"

namespace rev::test {
	namespace vertex {
		struct skybox {
			int8_t pos[3];

			const static FWVDecl s_vdecl;
		};
	}
	class Skybox {
		private:
			using Prim = SingletonData<Primitive, Skybox, 0>;
			using Tech = SingletonData<ITech, Skybox, 1>;
			using DefaultTex = SingletonData<GLTexture, Skybox, 2>;
			Prim			_primitive;
			Tech			_tech;
			DefaultTex		_defaultTex;
			HTexC			_texture;

		public:
			static HPrim	MakeData(Prim*);
			static HTech	MakeData(Tech*);
			static HTex		MakeData(DefaultTex*);

			void setTexture(const HTexC& t);
			void draw(IEffect& e) const;
	};
}
