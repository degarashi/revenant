#pragma once
#ifdef RESMGRNAME_DEFINED
	#error Include resmgr_app.hpp before spine/resmgr_named.hpp
#endif
#include "apppath.hpp"

namespace rev {
	struct URIWrap {
		HURI		uri;
		URIWrap() = default;
		URIWrap(const URI& uri):
			uri(uri.clone())
		{}
		bool operator == (const URIWrap& w) const noexcept {
			return *uri == *w.uri;
		}
	};
	inline std::ostream& operator << (std::ostream& os, const URIWrap& w) {
		return os << *w.uri;
	}
}
namespace spi {
	rev::URIWrap MakeAnonymous(rev::URIWrap*, uint64_t num);
	bool IsAnonymous(const rev::URIWrap& key);
}

#include "spine/src/resmgr_named.hpp"
namespace rev {
	template <class Dat>
	class ResMgrApp : public spi::ResMgrName<Dat, URIWrap> {
		private:
			using base_t = spi::ResMgrName<Dat, URIWrap>;
			AppPathCache	_cache;
			int				_idResType = 0;

			template <class Ar, class Dat2>
			friend void serialize(Ar&, ResMgrApp<Dat2>&);
		protected:
			template <class T>
			ResMgrApp(T&& t):
				_cache(std::forward<T>(t))
			{}
			void _setResourceTypeId(const int id) {
				_idResType = id;
			}
			// 名前オンリーのURI(=リソース名)を有効なURIに置き換え
			// (ResMgrへの登録名はURIで行う)
			void _modifyResourceName(URIWrap& key) const override {
				if(key.uri->getType() == URI::Type::User) {
					const auto& k = static_cast<const UserURI&>(*key.uri);
					key.uri = _cache.uriFromResourceName(_idResType, k.getName());
				}
			}
		public:
			// (acquireWithMakeをそのまま呼ぶと名前がわかりにくい為)
			// Make: args=(const URIWrap&, Constructor<T2>&)
			template <class T2, class Make>
			auto loadResourceApp(const URI& key, Make&& mk) {
				return base_t::template acquireWithMake<T2>(key, mk);
			}
			bool deepCmp(const ResMgrApp& r) const noexcept {
				return static_cast<const base_t&>(*this).deepCmp(r) &&
					_cache == r._cache &&
					_idResType == r._idResType;
			}
	};
}
namespace std {
	template <>
	struct hash<rev::URIWrap> {
		std::size_t operator()(const rev::URIWrap& w) const noexcept {
			return w.uri->getHash();
		}
	};
}
