#pragma once
#include "../vertex.hpp"
#include "../gl_types.hpp"
#include "handle/opengl.hpp"
#include "json_types.hpp"

namespace rev::gltf {
	using VSemCount = std::size_t[VSemEnum::_Num];
	struct PrimitiveVertex {
		constexpr static auto MaxBuff = sizeof(float)*4;
		using Buff = std::array<uint8_t, MaxBuff>;

		GLenum			type;
		VSemantic		vsem;
		bool			normalized;
		std::size_t		nElem,
						stride;
		Buff			value;

		template <class T, std::size_t N>
		PrimitiveVertex(
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
	using PrimitiveVertexV = std::vector<PrimitiveVertex>;
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
		mutable HPrim		primitive_cache;

		Primitive(const JValue& v, const Query_t& q);
		static bool CanLoad(const JValue& v) noexcept;
		const HPrim& getPrimitive() const;
	};
}
