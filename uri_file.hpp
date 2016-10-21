#pragma once
#include "urihandler.hpp"
#include "path.hpp"

namespace rev {
	//! ファイルシステムからのファイル読み込み
	class URI_File : public URIHandler {
		private:
			PathBlock		_basePath;
			HRW _openURI(const PathBlock& pb, int access);
		public:
			URI_File(To32Str path);
			static bool Capable(const URI& uri, int access);
			HRW openURI(const URI& uri, int access) override;
	};
}
