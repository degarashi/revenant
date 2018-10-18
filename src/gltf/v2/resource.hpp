#pragma once
#include "common.hpp"
#include "spine/src/enum.hpp"
#include "../json_types.hpp"

namespace rev::gltf::v2 {
	#define SEQ_V2_RES0 \
		(Accessor) \
		(Animation) \
		(Buffer) \
		(BufferView) \
		(Camera) \
		(Image) \
		(Material) \
		(Mesh) \
		(Sampler) \
		(Scene) \
		(Skin) \
		(Texture)
	#define SEQ_V2_RES1	SEQ_V2_RES0(AnimSampler)
	#define SEQ_V2_RES		SEQ_V2_RES1(Node)

	struct Resource {
		DefineEnum(
			Type,
			SEQ_V2_RES
		);
		spi::Optional<Name>	name;

		Resource(std::nullptr_t);
		Resource(const JValue& v);
		virtual ~Resource() {}
		virtual Type getType() const noexcept = 0;
		static bool CanLoad(const JValue& v) noexcept;
		const Name& getName() const noexcept;
	};
	#define DEF_RES(z, ign, name)	\
		class name; \
		using BOOST_PP_CAT(name, _OP) = spi::Optional<name>;
	BOOST_PP_SEQ_FOR_EACH(DEF_RES, EMPTY, SEQ_V2_RES)
	#undef DEF_RES
}
