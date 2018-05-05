#pragma once
#include "spine/enum.hpp"
#include "spine/optional.hpp"
#include "spine/flyweight_item.hpp"
#include "json_types.hpp"
#include <string>
#include <vector>

namespace rev::gltf {
	using GLSLName = std::string;
	using GLSLNameV = std::vector<GLSLName>;
	using Name = std::string;
	using SName = spi::FlyweightItem<std::string>;
	using SName_V = std::vector<SName>;
	using ByteV = std::vector<uint8_t>;
	using ByteV_OP = spi::Optional<ByteV>;
	using MIME = std::string;

	struct Resource {
		DefineEnum(
			Type,
			(AnimSampler)
			(Localfile)

			(Accessor)
			(Animation)
			(Buffer)
			(BufferView)
			(Camera)
			(Image)
			(Material)
			(Mesh)
			(Node)
			(Program)
			(Sampler)
			(Scene)
			(Shader)
			(Skin)
			(Technique)
			(Texture)
		);

		virtual ~Resource() {}
		Resource(const JValue& v);
		SName					name;
		spi::Optional<Name>		username;
		virtual Type getType() const noexcept = 0;
	};
	using Void_SP = std::shared_ptr<void>;
}
