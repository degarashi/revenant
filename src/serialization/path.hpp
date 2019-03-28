#pragma once
#include "../fs/path.hpp"
#include <cereal/types/string.hpp>

namespace rev {
	// UTF-8文字列として読み書き
	template <class Ar>
	void save(Ar& ar, const PathBlock& pb) {
		ar(pb.plain_utf8());
	}
	template <class Ar>
	void load(Ar& ar, PathBlock& pb) {
		std::string path;
		ar(path);
		pb.setPath(path);
	}
}
