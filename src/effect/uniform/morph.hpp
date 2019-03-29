#pragma once
#include "../../gltf/v2/morph.hpp"
#include "../uniform_setter.hpp"
#include "frea/src/vector.hpp"

namespace rev {
	class U_Morph :
		public IUniformSetter
	{
		public:
			struct Coeff {
				using Ar_Vec = std::array<frea::Vec4, gltf::v2::N_Morph/4>;
				union Ar_Union {
					gltf::v2::MorphCoeffMask		ar_f;
					Ar_Vec							ar_v;

					void fillZero();
				};
				Ar_Union	pos,
							normal,
							tangent;

				void fillZero();
			} coeff;

			UniformSetF getUniformF(const GLProgram& prog) const override;
	};
}
