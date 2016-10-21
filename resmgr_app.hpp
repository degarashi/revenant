#pragma once
#include "apppath.hpp"
#include "spine/resmgr_named.hpp"

namespace rev {
	template <class Dat>
	class ResMgrApp {
		private:
			class RM : public spi::ResMgrName<Dat, URI> {
				protected:
					void _modifyResourceName(URI& key) const override {
						// Protocolを持っていないリソース名を有効なURIに置き換え (ResMgrへの登録名はURIで行う)
						if(key.getType_utf8().empty())
							key = _cache.uriFromResourceName(_idResType, key.path().plain_utf8());
					}
			};
			RM				_mgr;
			AppPathCache	_cache;
			int				_idResType = 0;

			friend class cereal::access;
			template <class Ar>
			void serialize(Ar& ar) {
				ar(_mgr, _cache, _idResType);
			}
		protected:
			template <class T>
			ResMgrApp(T&& t):
				_cache(std::forward<T>(t))
			{}
			void _setResourceTypeId(const int id) {
				_idResType = id;
			}
		public:
			// CB = function<Dat (const URI&)>
			// INIT = function<void (Resource)>
			template <class KEY, class CB, class INIT>
			auto loadResourceApp(KEY&& name, CB&& cb, INIT&& cbInit) {
				const auto ret = _mgr.acquireWithMake(std::forward<KEY>(name), cb);
				if(ret.second)
					cbInit(ret.first.get());
				return ret;
			}
	};
}
