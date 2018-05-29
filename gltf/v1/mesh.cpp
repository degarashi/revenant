#include "gltf/v1/mesh.hpp"
#include "../check.hpp"
#include "../../primitive.hpp"
#include "gltf/v1/accessor.hpp"
#include "../../gl_resource.hpp"
#include "../../gl_buffer.hpp"
#include "../../vdecl.hpp"
#include "gltf/v1/buffer.hpp"

namespace rev::gltf::v1 {
	namespace L = gltf::loader;
	Primitive::Primitive(const JValue& v, const IDataQuery& q):
		gltf::Primitive<DRef_Accessor, IDataQuery>(v, q),
		material(L::Required<DRef_Material>(v, "material", q))
	{}
	Mesh::Mesh(const JValue& v, const IDataQuery& q):
		Resource(v),
		primitive(L::OptionalDefault<L::Array<Prim>>(v, "primitives", {}, q))
	{}
	Resource::Type Mesh::getType() const noexcept {
		return Type::Mesh;
	}
}
