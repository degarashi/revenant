#pragma once
#include "singleton_data_lazy.hpp"

namespace rev {
	template <class T, class Maker, int Id>
	class SingletonData :
		public std::shared_ptr<T>,
		public SingletonDataLazy<T, Maker, Id>
	{
		private:
			using base_t = SingletonDataLazy<T, Maker, Id>;
			using SP = std::shared_ptr<T>;
		public:
			SingletonData():
				SP(base_t::GetData())
			{}
	};
}
