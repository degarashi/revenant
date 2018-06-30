#pragma once
#include "../vertex.hpp"
#include "../gl_types.hpp"
#include "../vdecl.hpp"
#include "handle/opengl.hpp"
#include "json_types.hpp"
#include "../dc/bbox.hpp"

namespace rev::gltf {
	using VSemCount = std::size_t[VSemEnum::_Num];
	struct DummyVertex {
		constexpr static auto MaxBuff = sizeof(float)*4;
		using Buff = std::array<uint8_t, MaxBuff>;

		GLenum			type;
		VSemantic		vsem;
		bool			normalized;
		std::size_t		nElem,
						stride;
		Buff			value;

		template <class T, std::size_t N>
		DummyVertex(
			const GLenum type,
			const VSemantic vsem,
			const bool normalized,
			const std::size_t nElem,
			const T (&t)[N]
		):
			type(type),
			vsem(vsem),
			normalized(normalized),
			nElem(nElem)
		{
			setValue(t);
		}

		template <class T, std::size_t N>
		void setValue(const T (&t)[N]) {
			D_Assert0(N <= MaxBuff);
			auto* dst = reinterpret_cast<T*>(value.data());
			for(std::size_t i=0 ; i<N ; i++)
				*dst++ = t[i];
			stride = sizeof(T)*N;
		}
	};
	using DummyVertexV = std::vector<DummyVertex>;
	template <class Policy>
	struct Primitive {
		using Accessor_t = typename Policy::Accessor_t;
		using Query_t = typename Policy::Query_t;
		using VSem_t = typename Policy::VSem_t;
		using VSem_Accessor = std::vector<std::pair<VSemantic, Accessor_t>>;
		using Idx_Accessor_OP = spi::Optional<Accessor_t>;

		VSem_Accessor		attribute;
		Idx_Accessor_OP		index;
		DrawMode			mode;

		virtual void _getDummyVertex(const VSemCount& vc, DummyVertexV& v) const;
		virtual ~Primitive() {}

		mutable struct {
			using Prim_Op = spi::Optional<HPrim>;
			HPrim		normal;
			Prim_Op		tangent;
			dc::BBox_Op	bbox;
			bool		noBBox;
		} cache;
		static HVb ProcDummyVertex(VDecl::VDInfoV& vdinfo, std::size_t streamId, std::size_t nV, const DummyVertexV& v);

		Primitive(const JValue& v, const Query_t& q);
		static bool CanLoad(const JValue& v) noexcept;
		const HPrim& getPrimitive() const;
		const HPrim& getPrimitiveWithTangent() const;
		dc::BBox_Op getBBox() const;
	};
}
