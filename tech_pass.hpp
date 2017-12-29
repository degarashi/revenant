#pragma once
#include "tech_pair.hpp"
#include "gl_types.hpp"

namespace rev {
	class URI;
	class TechPass : public IGLResource {
		private:
			// [Tech|Pass] -> [TechId, PassId]
			using NameToId = std::unordered_map<Name, std::pair<int, int>>;
			NameToId	_nameToId;
			TechPairV	_tech;

		public:
			// ----------------- Exceptions -----------------
			//! 該当するGLXファイルが見つからない
			struct EC_FileNotFound : std::runtime_error {
				EC_FileNotFound(const std::string& fPath);
			};
			//! Effectファイル(gfx)を読み込む
			TechPass(const std::string& path);
			Tech_SP getTechnique(const Name& techpass) const;
			Tech_SP getTechnique(const Name& tech, const Name& pass) const;
	};
}
