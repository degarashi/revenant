#pragma once
#include "../resmgr_app.hpp"
#include "gltf.hpp"

namespace rev::gltf {
	using GLTF_SP = std::shared_ptr<GLTF>;
	#define mgr_gltf (::rev::gltf::GLTFMgr::ref())
	class GLTFMgr:
		public ResMgrApp<GLTF>,
		public spi::Singleton<GLTFMgr>
	{
		private:
			const static std::string cs_rtname[1];
		public:
			GLTFMgr();
			GLTF_SP loadGLTf(const URI& uri);
	};
}
