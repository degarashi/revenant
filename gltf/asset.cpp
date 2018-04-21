#include "gltf/asset.hpp"
#include "gltf/value_loader.hpp"

namespace rev {
	namespace gltf {
		using namespace loader;
		// ------------------ Asset::Profile ------------------
		Asset::Profile::Profile(const JValue& v):
			api(Optional<String>(v, "api", "WebGL")),
			version(Optional<String>(v, "version", "1.0.3"))
		{}

		// ------------------ Asset ------------------
		Asset::Asset(const JValue& v):
			copyright(Optional<String>(v, "copyright", "")),
			generator(Optional<String>(v, "generator", "")),
			version(Required<String>(v, "version")),
			premultipliedAlpha(Optional<Bool>(v, "premultipliedAlpha", false)),
			profile(Optional<Profile>(v, "profile"))
		{}
	}
}
