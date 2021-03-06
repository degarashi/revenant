#pragma once
#include "spine/src/enum.hpp"
#include "spine/src/optional.hpp"
#include "spine/src/flyweight_item.hpp"
#include "../json_types.hpp"

namespace rev::gltf::v1 {
	using Name = std::string;
	using SName = spi::FlyweightItem<std::string>;
	using SName_V = std::vector<SName>;
	using GLSLName = SName;
	using GLSLNameV = std::vector<GLSLName>;
	using MIME = std::string;

	#define SEQ_V1_RES0 \
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
	#define SEQ_V1_RES1	SEQ_V1_RES0(AnimSampler)
	#define SEQ_V1_RES		SEQ_V1_RES1(Node)

	struct Resource {
		DefineEnum(
			Type,
			SEQ_V1_RES
		);
		spi::Optional<Name>		username;

		static struct Identity_t {} Identity;
		Resource(Identity_t);
		Resource(const JValue& v);
		virtual ~Resource() {}
		virtual Type getType() const noexcept = 0;
		const Name& getName() const noexcept;
	};

	#define DEF_RES(z, ign, name) \
		struct name; \
		using BOOST_PP_CAT(name, _OP) = spi::Optional<name>;
	BOOST_PP_SEQ_FOR_EACH(DEF_RES, EMPTY, SEQ_V1_RES)
	#undef DEF_RES
}
