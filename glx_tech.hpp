#pragma once
#include "glx_parse.hpp"
#include "tech.hpp"

namespace rev {
	namespace parse {
		class BlockSet;
		using BlockSet_SP = std::shared_ptr<BlockSet>;
	}
	// Tech | Pass の分だけ作成
	class GLXTech : public Tech {
		private:
			parse::BlockSet_SP	_block;
		public:
			//! エフェクトファイルのパース結果を読み取る
			GLXTech(const parse::BlockSet_SP& bs, const parse::TPStruct& tech, const parse::TPStruct& pass);
	};
}
