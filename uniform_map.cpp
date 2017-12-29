#include "glx.hpp"
#include "uniform_pool.hpp"

namespace rev {
	draw::TokenBuffer* UniformMap::makeTokenBuffer(const GLint id) {
		const auto itr = _map.find(id);
		if(itr == _map.end()) {
			return _map[id] = unif_pool.allocate();
		}
		return itr->second;
	}
	void UniformMap::clear() {
		if(UnifPool::Initialized()) {
			auto& pool = unif_pool;
			for(auto& p : _map)
				pool.destroy(p.second);
		}
		_map.clear();
	}
	void UniformMap::copyFrom(const UniformMap& other) {
		for(auto& o : other._map) {
			auto* buff = makeTokenBuffer(o.first);
			o.second->clone(*buff);
		}
	}
	void UniformMap::moveTo(draw::TokenML& ml) {
		auto& pool = unif_pool;
		for(auto& u : _map) {
			u.second->takeout(ml);
			pool.destroy(u.second);
		}
		_map.clear();
	}
	bool UniformMap::empty() const noexcept {
		return _map.empty();
	}
	UniformMap::~UniformMap() {
		clear();
	}
}
