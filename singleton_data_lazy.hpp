#pragma once
#include <memory>

namespace rev {
	template <class T, class Maker>
	class SingletonDataLazy {
		private:
			using SP = std::shared_ptr<T>;
			using WP = std::weak_ptr<T>;

			static WP	s_wp;

		public:
			static SP GetData() {
				SP ret = s_wp.lock();
				if(!ret) {
					ret = Maker::MakeData();
					s_wp = ret;
				}
				return ret;
			}
	};
	template <class T, class Maker>
	typename SingletonDataLazy<T,Maker>::WP SingletonDataLazy<T,Maker>::s_wp;
}
