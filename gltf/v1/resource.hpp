#pragma once
#include "spine/enum.hpp"
#include "spine/optional.hpp"
#include "spine/flyweight_item.hpp"
#include "../json_types.hpp"

namespace rev::gltf::v1 {
	using Name = std::string;
	using SName = spi::FlyweightItem<std::string>;
	using SName_V = std::vector<SName>;
	using GLSLName = SName;
	using GLSLNameV = std::vector<GLSLName>;
	using MIME = std::string;

	#define SEQ_RES0 \
		(Accessor) \
		(Animation) \
		(Buffer) \
		(BufferView) \
		(Camera) \
		(Image) \
		(Mesh) \
		(Material) \
		(Program) \
		(Sampler) \
		(Scene) \
		(Skin) \
		(Shader) \
		(Technique) \
		(Texture)
	#define SEQ_RES1	SEQ_RES0(AnimSampler)
	#define SEQ_RES		SEQ_RES1(Node)

	struct Resource {
		DefineEnum(
			Type,
			SEQ_RES
		);
		spi::Optional<Name>		username;

		Resource(const JValue& v);
		virtual ~Resource() {}
		virtual Type getType() const noexcept = 0;
	};

	#define DEF_RES(z, ign, name) \
		struct name; \
		using BOOST_PP_CAT(name, _OP) = spi::Optional<name>;
	BOOST_PP_SEQ_FOR_EACH(DEF_RES, EMPTY, SEQ_RES)
	#undef DEF_RES
}
