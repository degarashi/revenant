#pragma once
#include "glx_parse.hpp"
#include "tech.hpp"

namespace rev {
	namespace parse {
		class BlockSet;
	}
	// Tech | Pass の分だけ作成
	class GLXTech : public Tech {
		public:
			//! エフェクトファイルのパース結果を読み取る
			GLXTech(const parse::BlockSet& bs, const parse::TPStruct& tech, const parse::TPStruct& pass);
	};
}
