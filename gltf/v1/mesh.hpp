#pragma once
#include "resource.hpp"
#include "gltf/v1/dataref.hpp"
#include "../../gl_format.hpp"
#include "../../gl_types.hpp"
#include "gltf/v_semantic.hpp"
#include "handle/opengl.hpp"

namespace rev::gltf::v1 {
	struct Mesh :
		Resource
	{
		struct Primitive {
			using VSem_Accessor = std::vector<std::pair<VSemantic, DRef_Accessor>>;
			using Idx_Accessor = spi::Optional<DRef_Accessor>;

			VSem_Accessor	attribute;
			Idx_Accessor	index;
			DrawMode		mode;
			DRef_Material	material;
			mutable HPrim	primitive_cache;

			Primitive(const JValue& v, const IDataQuery& q);
			static bool CanLoad(const JValue& v) noexcept;
			const HPrim& getPrimitive() const;
		};
		using PrimitiveV = std::vector<Primitive>;

		PrimitiveV	primitive;

		Mesh(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;
	};
}
