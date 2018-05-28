#pragma once
#include "gltf/exceptions.hpp"
#include "../path.hpp"

namespace rev::gltf {
	using std::literals::operator""s;
	using Path = std::string;
	struct IDataQueryBase {
		virtual PathBlock getFilePath(const Path& p) const {
			throw UnknownID("Unknown id: "s + p);
		}
		virtual ~IDataQueryBase() {}
	};
}
