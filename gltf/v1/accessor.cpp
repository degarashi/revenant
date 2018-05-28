#include "gltf/v1/accessor.hpp"
#include "../check.hpp"

namespace rev::gltf::v1 {
	namespace {
		const GLenum c_componentType[] = {
			GL_BYTE,
			GL_UNSIGNED_BYTE,
			GL_SHORT,
			GL_UNSIGNED_SHORT,
			GL_FLOAT
		};
		struct ElemType {
			const std::string	name;
			bool				bMat;
			int					nElem;

			bool operator == (const char* c) const noexcept {
				return name == c;
			}
		};
		const ElemType c_elemtype[] = {
			{"SCALAR", false, 1},
			{"VEC2", false, 2},
			{"VEC3", false, 3},
			{"VEC4", false, 4},
			{"MAT2", true, 4},
			{"MAT3", true, 9},
			{"MAT4", true, 16}
		};
	}
	using namespace loader;
	Accessor::Accessor(const JValue& v, const IDataQuery& q):
		Resource(v),
		byteOffset(Required<Integer>(v, "byteOffset")),
		byteStride(OptionalDefault<Integer>(v, "byteStride", 0)),
		componentType(Required<Integer>(v, "componentType")),
		count(Required<Integer>(v, "count")),
		bufferView(Required<DRef_BufferView>(v, "bufferView", q))
	{
		CheckRange<std::size_t>(byteOffset, 0);
		CheckEnum(c_componentType, componentType);
		CheckRange<std::size_t>(count, 1);
		const auto& typ = CheckEnum(c_elemtype, Required<String>(v, "type"));

		const auto max = OptionalDefault<Number_A>(v, "max", {}),
					min = OptionalDefault<Number_A>(v, "min", {});
		const int sMax = max.size(),
					sMin = min.size();
		if(sMax > 0) {
			if(sMax != sMin ||
				sMax != typ.nElem)
				throw InvalidProperty("size(max) and size(min) differs");
			// filter.resize(sMax);
			// for(int i=0 ; i<sMax; i++) {
				// filter[i] = {min[i], max[i]};
			// }
		}
		bMatrix = typ.bMat;
		nElem = typ.nElem;
		CheckRange<std::size_t>(byteStride, 0, 255);
		if(byteStride == 0)
			byteStride = GLFormat::QuerySize(componentType) * nElem;
	}
	Resource::Type Accessor::getType() const noexcept {
		return Type::Accessor;
	}
	void Accessor::filterValue(double* data) const noexcept {
		if(filter.empty())
			return;
		const int n = count,
				ne = nElem;
		for(int i=0 ; i<n*ne ; i++) {
			const auto& mm = filter[i%ne];
			*data = std::min(std::max(mm.first, *data), mm.second);
			++data;
		}
	}
}
