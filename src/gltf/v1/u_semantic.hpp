#pragma once
#include "spine/src/enum.hpp"
#include "../../gl_header.hpp"
#include "spine/src/optional.hpp"

namespace rev::gltf::v1 {
	DefineEnum(USemantic,
		(Local)
		(Model)
		(View)
		(Projection)
		(ModelView)
		(ModelViewProjection)
		(ModelInverse)
		(ViewInverse)
		(ProjectionInverse)
		(ModelViewInverse)
		(ModelViewProjectionInverse)
		(ModelInverseTranspose)
		(ModelViewInverseTranspose)
		(Viewport)
		(JointMatrix)
	);
	struct U_Semantic {
		std::string		sem;
		GLenum			type;

		static spi::Optional<USemantic> FromString(const char* s);
		bool operator == (const char* s) const noexcept;
	};
}
