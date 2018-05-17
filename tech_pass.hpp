#pragma once
#include "tech_pair.hpp"
#include "resource.hpp"

namespace rev {
	class TechPass : public Resource {
		private:
			// [Tech|Pass] -> HTech
			using NameToTech = std::unordered_map<Name, HTech>;
			NameToTech		_nameToTech;

		public:
			static Name MakeName(const Name& tech, const Name& pass);
			// ----------------- Exceptions -----------------
			//! 該当するGLXファイルが見つからない
			struct EC_FileNotFound : std::runtime_error {
				EC_FileNotFound(const std::string& fPath);
			};
			//! Effectファイル(gfx)を読み込む
			TechPass(const std::string& path);
			HTech getTechnique(const Name& techpass) const;
			HTech getTechnique(const Name& tech, const Name& pass) const;
			const char* getResourceName() const noexcept override;
	};
}
