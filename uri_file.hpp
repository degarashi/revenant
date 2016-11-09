#pragma once
#include "urihandler.hpp"
#include "path.hpp"

namespace cereal {
	template <class T>
	struct LoadAndConstruct;
}
namespace rev {
	//! ファイルシステムからのファイル読み込み
	class URI_File : public URIHandler {
		private:
			PathBlock		_basePath;
			HRW _openURI(const PathBlock& pb, int access);

			template <class Ar>
			friend void serialize(Ar&, URI_File&);
			template <class T>
			friend struct cereal::LoadAndConstruct;
		public:
			URI_File(To32Str path);
			static bool Capable(const URI& uri, int access);
			HRW openURI(const URI& uri, int access) override;
	};
}