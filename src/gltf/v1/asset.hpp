#pragma once
#include "resource.hpp"

namespace rev::gltf::v1 {
	struct Asset {
		std::string		copyright,
						generator,
						version;
		bool			premultipliedAlpha;

		struct Profile {
			std::string		api,
							version;

			Profile(const JValue& v);
		};
		using Profile_OP = spi::Optional<Profile>;
		Profile_OP		profile;

		Asset(const JValue& v);
	};
}
