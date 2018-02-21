#pragma once
#include <memory>
#include "lubee/meta/constant_t.hpp"

namespace rev {
	template <class T, class Maker, int Id>
	class SingletonDataLazy {
		private:
			using SP = std::shared_ptr<T>;
			using WP = std::weak_ptr<T>;

			static WP	s_wp;

		public:
			static SP GetData() {
				SP ret = s_wp.lock();
				if(!ret) {
					ret = Maker::MakeData(lubee::IConst<Id>());
					s_wp = ret;
				}
				return ret;
			}
	};
	template <class T, class Maker, int Id>
	typename SingletonDataLazy<T,Maker,Id>::WP SingletonDataLazy<T,Maker,Id>::s_wp;
}
