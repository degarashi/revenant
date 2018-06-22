#pragma once
#include "gltf/v2/dataref.hpp"
#include "gltf/v2/v_semantic.hpp"
#include "gltf/v2/morph.hpp"
#include "gltf/primitive.hpp"
#include "../rwref.hpp"

namespace rev::gltf::v2 {
	struct PrimitivePolicy {
		using Accessor_t = DRef_Accessor;
		using Query_t = IDataQuery;
		using VSem_t = V_Semantic;
	};
	struct Primitive :
		gltf::Primitive<PrimitivePolicy>
	{
		using Morph_Op = spi::Optional<Morph>;

		DRef_Material_OP	material;
		Morph_Op			morph;

		void _getDummyVertex(const VSemCount& vc, DummyVertexV& v) const override;

		Primitive(const JValue& v, const IDataQuery& q);
	};

	using WeightV_S = std::shared_ptr<WeightV>;
	class Mesh :
		public Resource
	{
		private:
			using PrimitiveV = std::vector<Primitive>;

		public:
			PrimitiveV	primitives;
			WeightV_S	weight;

			Mesh(const JValue& v, const IDataQuery& q);
			Type getType() const noexcept override;
	};
}
