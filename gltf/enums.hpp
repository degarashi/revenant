#pragma once
#include "spine/enum.hpp"
#include "../gl_header.hpp"
#include "../vertex.hpp"
#include "spine/optional.hpp"

namespace rev::gltf {
	struct V_Semantic {
		std::string		sem;

		static spi::Optional<VSemantic> FromString(const char* s);
		bool operator == (const std::string& s) const noexcept;
		bool operator == (const char* s) const noexcept;
	};
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
