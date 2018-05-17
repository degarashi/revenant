#pragma once
#include "resmgr_app.hpp"
#include "glx_parse.hpp"
#include "handle/opengl.hpp"
#include "tech_pass.hpp"

namespace rev::parse {
	class BlockSet;
	#define mgr_tech (::rev::parse::TechMgr::ref())
	class TechMgr : public ResMgrApp<GLXStruct>, public spi::Singleton<TechMgr> {
		private:
			using Path = std::string;
			BlockSet _loadBlockSet(const Path& path);
		public:
			TechMgr();
			HTP loadTechPass(const Path& path);
	};
}
