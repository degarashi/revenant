#pragma once
#include "apppath.hpp"
#include "spine/resmgr_named.hpp"

namespace rev {
	template <class Dat>
	class ResMgrApp : public spi::ResMgrName<Dat, URI> {
		private:
			using base_t = spi::ResMgrName<Dat, URI>;
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
			void _modifyResourceName(URI& key) const override {
				// Protocolを持っていないリソース名を有効なURIに置き換え (ResMgrへの登録名はURIで行う)
				if(key.getType_utf8().empty())
					key = _cache.uriFromResourceName(_idResType, key.path().plain_utf8());
			}
		public:
			// CB = function<Dat (const URI&)>
			// INIT = function<void (Resource)>
			template <class T2, class KEY, class CB>
			auto loadResourceApp(KEY&& name, CB&& cb) {
				return base_t::template acquireWithMake<T2>(std::forward<KEY>(name), cb);
			}
	};
}
