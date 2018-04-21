#include "mgr.hpp"
#include "../uri.hpp"

namespace rev::gltf {
	const std::string GLTFMgr::cs_rtname[] = {
		"gltf",
	};
	GLTFMgr::GLTFMgr():
		ResMgrApp(cs_rtname)
	{}
	GLTF_SP GLTFMgr::loadGLTf(const URI& uri) {
		_setResourceTypeId(0);
		return loadResourceApp<GLTF>(uri,
			[](auto& uri, auto&& mk){
				mk(GLTF::Make(dynamic_cast<const FileURI&>(*uri.uri)));
			}
		).first;
	}
}
