#include "qm_cached.hpp"
#include "lubee/hash_combine.hpp"

namespace rev::dc {
	// ------------------- QMCached::IGet -------------------
	QMCached::IGet::~IGet() {}
	const Mat4& QMCached::IGet::get() const {
		D_Assert0(false); throw 0;
	}
	bool QMCached::IGet::operator == (const QMCached::IGet& g) const noexcept {
		const auto &typ0 = typeid(*this),
					&typ1 = typeid(g);
		if(typ0 == typ1) {
			if(typ0 == typeid(GetById)) {
				return static_cast<const GetById&>(*this) == static_cast<const GetById&>(g);
			} else {
				return static_cast<const GetByName&>(*this) == static_cast<const GetByName&>(g);
			}
		}
		return false;
	}
	// ------------------- QMCached::GetById -------------------
	QMCached::GetById::GetById(const JointId id, const bool local):
		id(id),
		local(local)
	{}
	void QMCached::GetById::refresh(const IQueryMatrix& q) {
		result = local ? q.getLocal(id) : q.getGlobal(id);
	}
	const Mat4& QMCached::GetById::get() const {
		return result;
	}
	std::size_t QMCached::GetById::getHash() const noexcept {
		return lubee::hash_combine_implicit(id, local);
	}
	bool QMCached::GetById::operator == (const GetById& g) const noexcept {
		return id==g.id && local==g.local;
	}
	// ------------------- QMCached::GetByName -------------------
	QMCached::GetByName::GetByName(const SName& name, const bool local):
		name(name),
		local(local)
	{}
	void QMCached::GetByName::refresh(const IQueryMatrix& q) {
		result = local ? q.getLocal(name) : q.getGlobal(name);
	}
	const Mat4& QMCached::GetByName::get() const {
		return result;
	}
	std::size_t QMCached::GetByName::getHash() const noexcept {
		return lubee::hash_combine_implicit(name, local);
	}
	bool QMCached::GetByName::operator == (const GetByName& g) const noexcept {
		return name==g.name && local==g.local;
	}

	// ------------------- QMCached::Hash -------------------
	std::size_t QMCached::Hash::operator()(const IGet* g) const noexcept {
		return g->getHash();
	}
	// ------------------- QMCached::Equal -------------------
	bool QMCached::Equal::operator()(const IGet* g0, const IGet* g1) const noexcept {
		return *g0 == *g1;
	}
	// ------------------- QMCached::CacheUse -------------------
	QMCached::CacheUse::CacheUse(const CachePtrV& cache):
		_cache(cache)
	{}
	void QMCached::CacheUse::resetCursor() const {
		_cursor = 0;
	}
	const Mat4& QMCached::CacheUse::_getMat4() const {
		D_Assert0(_cursor < _cache.size());
		return _cache[_cursor++]->get();
	}
	Mat4 QMCached::CacheUse::getLocal(const JointId) const {
		return _getMat4();
	}
	Mat4 QMCached::CacheUse::getGlobal(const JointId) const {
		return _getMat4();
	}
	Mat4 QMCached::CacheUse::getLocal(const SName&) const {
		return _getMat4();
	}
	Mat4 QMCached::CacheUse::getGlobal(const SName&) const {
		return _getMat4();
	}

	// ------------------- QMCached -------------------
	QMCached::QMCached(const IQueryMatrix& q):
		_cacheUse(_cachePtr),
		_source(q),
		_initialized(false)
	{}
	const IQueryMatrix& QMCached::prepareInterface() {
		if(_initialized) {
			if(_cacheV.empty()) {
				for(auto& c : _cache)
					_cacheV.emplace_back(std::move(c.second));
				_cache.clear();
			}
			for(auto& p : _cacheV)
				p->refresh(_source);
			_cacheUse.resetCursor();
			return _cacheUse;
		}
		_initialized = true;
		return *this;
	}
	template <class Ent>
	void QMCached::_register(Ent ent) const {
		const auto itr = _cache.find(&ent);
		if(itr == _cache.end()) {
			auto p = std::make_unique<Ent>(std::move(ent));
			_cachePtr.emplace_back(p.get());
			_cache.emplace(p.get(), std::move(p));
		} else {
			_cachePtr.emplace_back(itr->first);
		}
	}
	Mat4 QMCached::getLocal(const JointId id) const {
		_register(GetById(id, true));
		return _source.getLocal(id);
	}
	Mat4 QMCached::getGlobal(const JointId id) const {
		_register(GetById(id, false));
		return _source.getGlobal(id);
	}
	Mat4 QMCached::getLocal(const SName& name) const {
		_register(GetByName(name, true));
		return _source.getLocal(name);
	}
	Mat4 QMCached::getGlobal(const SName& name) const {
		_register(GetByName(name, false));
		return _source.getGlobal(name);
	}
}
