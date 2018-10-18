#pragma once
#include "spine/src/optional.hpp"
#include "spine/src/flyweight_item.hpp"

namespace rev::dc {
	class TfNode;
	using JointId = uint32_t;
}
namespace rev::gltf::v2 {
	using Name = std::string;
	using SName = spi::FlyweightItem<std::string>;
	using SName_V = std::vector<SName>;
	using String = std::string;
	using String_OP = spi::Optional<std::string>;
	using Size = std::size_t;
	using Size_OP = spi::Optional<Size>;
	using MIME = std::string;
	using MIME_OP = spi::Optional<MIME>;
}
