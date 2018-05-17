#include "gl_resource.hpp"
#include "gl_texture.hpp"
#include "glx_tech.hpp"

namespace rev {
	HRes GLRes::loadResource(const URI& uri) {
		HRes ret;
		if(uri.getType() == URI::Type::File) {
			// とりあえず今はファイルのみ対応
			auto& fu = static_cast<const FileURI&>(uri);
			const auto ext = fu.pathblock().getExtension();
			// is it Texture?
			if(ext=="png" || ext=="jpg" || ext=="bmp")
				ret = loadTexture(uri);
			// is it TechPass(Effect)?
			else if(ext == "glx") {
				ret = mgr_tech.loadTechPass(fu.pathblock().plain_utf8());
			}
		}
		return ret;
	}
}
