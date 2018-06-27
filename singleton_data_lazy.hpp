#pragma once
#include <memory>
#include "lubee/meta/constant_t.hpp"

namespace rev {
	template <class T, class Maker, std::size_t Id=0>
	class SingletonDataLazy {
		private:
			using SP = std::shared_ptr<T>;
			using WP = std::weak_ptr<T>;

			static WP	s_wp;

		public:
			template <class Key = SingletonDataLazy>
			static SP GetData() {
				SP ret = s_wp.lock();
				if(!ret) {
					ret = Maker::MakeData((Key*)nullptr);
					s_wp = ret;
				}
				return ret;
			}
	};
	template <class T, class Maker, std::size_t Id>
	typename SingletonDataLazy<T,Maker,Id>::WP SingletonDataLazy<T,Maker,Id>::s_wp;
}
