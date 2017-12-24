#include "glx_parse.hpp"
#include "material.hpp"

namespace rev {
	namespace parse {
		class BlockSet;
		using BlockSet_SP = std::shared_ptr<BlockSet>;
	}
	// Tech | Pass の分だけ作成
	class GLXMaterial : public Material {
		public:
			using MacroMap = std::unordered_map<std::string, std::string>;
			using AttrL = std::vector<const parse::AttrEntry*>;
			using VaryL = std::vector<const parse::VaryEntry*>;
			using ConstL = std::vector<const parse::ConstEntry*>;
			using UnifL = std::vector<const parse::UnifEntry*>;
		private:
			parse::BlockSet_SP	_block;
			AttrL				_attrL;
			VaryL				_varyL;
			UnifL				_unifL;
			ConstL				_constL;
		protected:
			void _onDeviceReset(const IEffect& e, Material::Runtime&) override;
		public:
			//! エフェクトファイルのパース結果を読み取る
			GLXMaterial(const parse::BlockSet_SP& bs, const parse::TPStruct& tech, const parse::TPStruct& pass);
	};
}
