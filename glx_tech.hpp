#pragma once
#include "resmgr_app.hpp"
#include "glx_parse.hpp"
#include "tech_pass.hpp"
#include "handle/opengl.hpp"
#include <unordered_set>

namespace rev {
	namespace parse {
		class BlockSet : public std::unordered_set<HBlock> {
			public:
				spi::Optional<const AttrStruct&> findAttribute(const std::string& s) const;
				spi::Optional<const ConstStruct&> findConst(const std::string& s) const;
				spi::Optional<const UnifStruct&> findUniform(const std::string& s) const;
				spi::Optional<const VaryStruct&> findVarying(const std::string& s) const;
				spi::Optional<const ShStruct&> findShader(const std::string& s) const;
				spi::Optional<const TPStruct&> findTechPass(const std::string& s) const;
				spi::Optional<const CodeStruct&> findCode(const std::string& s) const;
		};
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
}
