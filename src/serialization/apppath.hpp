#pragma once
#include "../apppath.hpp"
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

namespace rev {
	template <class Ar>
	void save(Ar& ar, const AppPathCache& ap) {
		using SV = std::vector<std::string>;
		// リソース名だけシリアライズ
		SV sv;
		for(auto& s : ap._cache)
			sv.emplace_back(s.first);
		ar(cereal::make_nvp("resource_name", sv));
	}
	template <class Ar>
	void load(Ar& ar, AppPathCache& ap) {
		using SV = std::vector<std::string>;
		SV sv;
		ar(cereal::make_nvp("resource_name", sv));

		ap._init(sv.data(), sv.size());
	}
}
