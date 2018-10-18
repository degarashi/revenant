#include "qm_fixed.hpp"
#include "lubee/src/hash_combine.hpp"
#include "jointquery_if.hpp"
#include "dc/node.hpp"

namespace rev::dc {
	// ------------------- QM_Fixed::Ent -------------------
	QM_Fixed::Ent::Ent(const TfNode* node, const bool local, const std::size_t sid):
		node(node),
		local(local),
		serialId(sid)
	{}
	void QM_Fixed::Ent::refresh() const {
		if(local)
			result = node->getPose().getToWorld().convertI<4,4>(1);
		else
			result = node->getTransform();
	}
	bool QM_Fixed::Ent::operator == (const Ent& e) const noexcept {
		return node == e.node &&
				local == e.local;
	}

	// ------------------- QM_Fixed::CacheUse -------------------
	QM_Fixed::CacheUse::CacheUse(const CachePtrV& cache):
		_cache(cache)
	{}
	void QM_Fixed::CacheUse::resetCursor() const {
		_cursor = 0;
	}
	const Mat4& QM_Fixed::CacheUse::_getMat4() const {
		D_Assert0(_cursor < _cache.size());
		return _cache[_cursor++]->result;
	}
	Mat4 QM_Fixed::CacheUse::getLocal(const JointId) const {
		return _getMat4();
	}
	Mat4 QM_Fixed::CacheUse::getGlobal(const JointId) const {
		return _getMat4();
	}
	Mat4 QM_Fixed::CacheUse::getLocal(const SName&) const {
		return _getMat4();
	}
	Mat4 QM_Fixed::CacheUse::getGlobal(const SName&) const {
		return _getMat4();
	}
	// ------------------- QM_Fixed::Hash -------------------
	std::size_t QM_Fixed::Hash::operator()(const Ent& e) const noexcept {
		return lubee::hash_combine_implicit(e.node, e.local);
	}
	// ------------------- QM_Fixed -------------------
	QM_Fixed::QM_Fixed(const IJointQuery& q):
		_cacheUse(_cachePtr),
		_source(q),
		_serialId(0),
		_initialized(false)
	{}
	const IQueryMatrix& QM_Fixed::prepareInterface() {
		if(_initialized) {
			if(_cacheV.empty()) {
				_cacheV.resize(_cache.size());
				for(auto& c : _cache)
					_cacheV[c.serialId] = c;
				for(auto*& c : _cachePtr)
					c = &_cacheV[c->serialId];
				_cache.clear();
			}
			for(auto& p : _cacheV)
				p.refresh();

			_cacheUse.resetCursor();
			return _cacheUse;
		}
		_initialized = true;
		return *this;
	}
	void QM_Fixed::clearCache() {
		_cache.clear();
		_cacheV.clear();
		_cachePtr.clear();
		_serialId = 0;
		_initialized = false;
	}
	void QM_Fixed::_register(const TfNode* node, const bool local) const {
		Ent ent(node, local, ~0);
		auto itr = _cache.find(ent);
		if(itr == _cache.end()) {
			itr = _cache.emplace(Ent(node, local, _serialId++)).first;
		}
		_cachePtr.emplace_back(&(*itr));
	}
	template <class Key>
	Mat4 QM_Fixed::_getMat(const Key& key, const bool local) const {
		const TfNode* node = _source.queryJoint(key);
		_register(node, local);
		if(local)
			return node->getPose().getToWorld().convertI<4,4>(1);
		return node->getTransform();
	}
	Mat4 QM_Fixed::getLocal(const JointId id) const {
		return _getMat(id, true);
	}
	Mat4 QM_Fixed::getGlobal(const JointId id) const {
		return _getMat(id, false);
	}
	Mat4 QM_Fixed::getLocal(const SName& name) const {
		return _getMat(name, true);
	}
	Mat4 QM_Fixed::getGlobal(const SName& name) const {
		return _getMat(name, false);
	}
}
