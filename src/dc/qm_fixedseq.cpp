#include "qm_fixedseq.hpp"
#include "lubee/src/hash_combine.hpp"

namespace rev::dc {
	// ------------------- QM_FixedSequence::IGet -------------------
	QM_FixedSequence::IGet::~IGet() {}
	bool QM_FixedSequence::IGet::operator == (const QM_FixedSequence::IGet& g) const noexcept {
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
	// ------------------- QM_FixedSequence::GetById -------------------
	QM_FixedSequence::GetById::GetById(const JointId id, const bool local):
		id(id),
		local(local)
	{}
	void QM_FixedSequence::GetById::refresh(const IQueryMatrix& q) {
		result = local ? q.getLocal(id) : q.getGlobal(id);
	}
	std::size_t QM_FixedSequence::GetById::getHash() const noexcept {
		return lubee::hash_combine_implicit(id, local);
	}
	bool QM_FixedSequence::GetById::operator == (const GetById& g) const noexcept {
		return id==g.id && local==g.local;
	}
	// ------------------- QM_FixedSequence::GetByName -------------------
	QM_FixedSequence::GetByName::GetByName(const SName& name, const bool local):
		name(name),
		local(local)
	{}
	void QM_FixedSequence::GetByName::refresh(const IQueryMatrix& q) {
		result = local ? q.getLocal(name) : q.getGlobal(name);
	}
	std::size_t QM_FixedSequence::GetByName::getHash() const noexcept {
		return lubee::hash_combine_implicit(name, local);
	}
	bool QM_FixedSequence::GetByName::operator == (const GetByName& g) const noexcept {
		return name==g.name && local==g.local;
	}

	// ------------------- QM_FixedSequence::Hash -------------------
	std::size_t QM_FixedSequence::Hash::operator()(const IGet* g) const noexcept {
		return g->getHash();
	}
	// ------------------- QM_FixedSequence::Equal -------------------
	bool QM_FixedSequence::Equal::operator()(const IGet* g0, const IGet* g1) const noexcept {
		return *g0 == *g1;
	}
	// ------------------- QM_FixedSequence::CacheUse -------------------
	QM_FixedSequence::CacheUse::CacheUse(const CachePtrV& cache):
		_cache(cache)
	{}
	void QM_FixedSequence::CacheUse::resetCursor() const {
		_cursor = 0;
	}
	const Mat4& QM_FixedSequence::CacheUse::_getMat4() const {
		D_Assert0(_cursor < _cache.size());
		return _cache[_cursor++]->result;
	}
	Mat4 QM_FixedSequence::CacheUse::getLocal(const JointId) const {
		return _getMat4();
	}
	Mat4 QM_FixedSequence::CacheUse::getGlobal(const JointId) const {
		return _getMat4();
	}
	Mat4 QM_FixedSequence::CacheUse::getLocal(const SName&) const {
		return _getMat4();
	}
	Mat4 QM_FixedSequence::CacheUse::getGlobal(const SName&) const {
		return _getMat4();
	}

	// ------------------- QM_FixedSequence -------------------
	QM_FixedSequence::QM_FixedSequence(const IQueryMatrix& q):
		_cacheUse(_cachePtr),
		_source(q),
		_initialized(false)
	{}
	const IQueryMatrix& QM_FixedSequence::prepareInterface() {
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
	void QM_FixedSequence::_register(Ent ent) const {
		const auto itr = _cache.find(&ent);
		if(itr == _cache.end()) {
			auto p = std::make_unique<Ent>(std::move(ent));
			_cachePtr.emplace_back(p.get());
			_cache.emplace(p.get(), std::move(p));
		} else {
			_cachePtr.emplace_back(itr->first);
		}
	}
	Mat4 QM_FixedSequence::getLocal(const JointId id) const {
		_register(GetById(id, true));
		return _source.getLocal(id);
	}
	Mat4 QM_FixedSequence::getGlobal(const JointId id) const {
		_register(GetById(id, false));
		return _source.getGlobal(id);
	}
	Mat4 QM_FixedSequence::getLocal(const SName& name) const {
		_register(GetByName(name, true));
		return _source.getLocal(name);
	}
	Mat4 QM_FixedSequence::getGlobal(const SName& name) const {
		_register(GetByName(name, false));
		return _source.getGlobal(name);
	}
}
