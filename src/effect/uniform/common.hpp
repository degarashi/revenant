#pragma once
#include "../uniform_setter.hpp"
#include "../../handle/opengl.hpp"
#include "frea/src/vector.hpp"

namespace rev {
	class U_Common : public IUniformSetter {
		public:
			float			alpha;
			frea::Vec4		color;
			float			depth;
			struct {
				HTexC		diffuse,
							specular,
							normal,
							emissive;
			} texture;

			U_Common();
			UniformSetF getUniformF(const GLProgram& prog) const override;
	};
}
