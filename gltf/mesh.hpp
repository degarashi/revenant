#pragma once
#include "resource.hpp"
#include "idtag.hpp"
#include "../gl_format.hpp"
#include "../gl_types.hpp"
#include "gltf/enums.hpp"
#include "handle/opengl.hpp"

namespace rev::gltf {
	struct Mesh :
		Resource,
		IResolvable
	{
		struct Primitive : IResolvable {
			using VSem_Accessor = std::vector<std::pair<VSemantic, TagAccessor>>;
			using Idx_Accessor = spi::Optional<TagAccessor>;

			VSem_Accessor	attribute;
			Idx_Accessor	index;
			DrawMode		mode;
			TagMaterial		material;
			mutable FWPrim	primitive_cache;

			Primitive(const JValue& v);
			void resolve(const ITagQuery& q) override;
			static bool CanLoad(const JValue& v) noexcept;
			const FWPrim& getPrimitive() const;
		};
		using PrimitiveV = std::vector<Primitive>;

		PrimitiveV	primitive;

		Mesh(const JValue& v);
		Type getType() const noexcept override;
		void resolve(const ITagQuery& q) override;
	};
}
