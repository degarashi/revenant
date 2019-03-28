#pragma once
#include "parse.hpp"
#include "../handle/opengl.hpp"

namespace rev::parse {
	class BlockSet : public std::unordered_set<HBlock> {
		private:
			using Path_V = std::vector<std::string>;
		public:
			Path_V	dependency;
			spi::Optional<const AttrStruct&> findAttribute(const std::string& s) const;
			spi::Optional<const ConstStruct&> findConst(const std::string& s) const;
			spi::Optional<const UnifStruct&> findUniform(const std::string& s) const;
			spi::Optional<const VaryStruct&> findVarying(const std::string& s) const;
			spi::Optional<const ShStruct&> findShader(const std::string& s) const;
			spi::Optional<const TPStruct&> findTechPass(const std::string& s) const;
			spi::Optional<const CodeStruct&> findCode(const std::string& s) const;
	};
}
