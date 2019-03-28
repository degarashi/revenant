#include "file.hpp"
#include "../sdl/rw.hpp"
#include "../apppath.hpp"

namespace rev {
	// ---------------------------- URI_File ----------------------------
	URI_File::URI_File(To32Str path):
		_basePath(path.moveTo())
	{}
	bool URI_File::Capable(const URI& uri, int /*access*/) {
		return uri.getType() == URI::Type::File;
	}
	HRW URI_File::_openURI(const PathBlock& path, int access) const {
		// 相対パス時はプログラムが置いてある地点から探索
		if(!path.isAbsolute()) {
			PathBlock pb(mgr_path.getAppDir());
			pb <<= path;
			return _openURI(pb, access);
		}
		try {
			return mgr_rw.fromFile(path, access);
		} catch(const std::exception& e) {
			// ファイルが読み込めないのはエラーとして扱わない
		}
		return nullptr;
	}
	HRW URI_File::openURI(const URI& uri, int access) const {
		if(Capable(uri, access)) {
			auto& fu = static_cast<const FileURI&>(uri);
			return _openURI(fu.pathblock(), access);
		}
		return nullptr;
	}
}
