#pragma once
#include <unordered_map>
#include <functional>

namespace rev {
	template <class K,
			 class V,
			 class Hash = std::hash<K>,
			 class Cmp = std::equal_to<>,
			 class Alc = std::allocator<std::pair<const K, V>> >
	class Cache : public std::unordered_map<K, V, Hash, Cmp, Alc> {
		private:
			using MakeF = std::function<V (const K&)>;
			MakeF	_make;
		public:
			Cache(const MakeF& m):
				_make(m)
			{}
			const V& getCache(const K& key) {
				auto itr = this->find(key);
				if(itr == this->end())
					itr = this->emplace(key, _make(key)).first;
				return itr->second;
			}
	};
}
