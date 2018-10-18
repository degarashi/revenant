#include "gltf/v2/asset.hpp"
#include "../value_loader.hpp"

namespace rev::gltf::v2 {
	namespace L = gltf::loader;

	Asset::Asset(const JValue& v):
		copyright(L::Optional<L::String>(v, "copyright")),
		generator(L::Optional<L::String>(v, "generator")),
		minVersion(L::Optional<L::String>(v, "minVersion")),
		version(L::Required<L::String>(v, "version"))
	{}
}
