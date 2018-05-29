#pragma once
#include "resource.hpp"
#include "gltf/v1/dataref.hpp"
#include "gltf/primitive.hpp"
#include "../../gl_format.hpp"

namespace rev::gltf::v1 {
	struct Primitive :
		gltf::Primitive<DRef_Accessor, IDataQuery>
	{
		DRef_Material	material;

		Primitive(const JValue& v, const IDataQuery& q);
	};
	struct Mesh :
		Resource
	{
		using Prim = Primitive;
		using PrimV = std::vector<Prim>;
		PrimV		primitive;

		Mesh(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;
	};
}
