#pragma once
#include "v_semantic.hpp"
#include "../gl_types.hpp"
#include "handle/opengl.hpp"
#include "json_types.hpp"

namespace rev::gltf {
	template <class D_Accessor, class Q>
	struct Primitive {
		using VSem_Accessor = std::vector<std::pair<VSemantic, D_Accessor>>;
		using Idx_Accessor_OP = spi::Optional<D_Accessor>;

		VSem_Accessor		attribute;
		Idx_Accessor_OP		index;
		DrawMode			mode;
		mutable HPrim		primitive_cache;

		Primitive(const JValue& v, const Q& q);
		static bool CanLoad(const JValue& v) noexcept;
		const HPrim& getPrimitive() const;
	};
}
