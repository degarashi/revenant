#include "gltf/v1/asset.hpp"
#include "gltf/v1/value_loader.hpp"

namespace rev {
	namespace gltf::v1 {
		using namespace loader;
		// ------------------ Asset::Profile ------------------
		Asset::Profile::Profile(const JValue& v):
			api(OptionalDefault<String>(v, "api", "WebGL")),
			version(OptionalDefault<String>(v, "version", "1.0.3"))
		{}

		// ------------------ Asset ------------------
		Asset::Asset(const JValue& v):
			copyright(OptionalDefault<String>(v, "copyright", "")),
			generator(OptionalDefault<String>(v, "generator", "")),
			version(Required<String>(v, "version")),
			premultipliedAlpha(OptionalDefault<Bool>(v, "premultipliedAlpha", false)),
			profile(Optional<Profile>(v, "profile"))
		{}
	}
}
