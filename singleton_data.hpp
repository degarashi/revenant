#pragma once
#include "singleton_data_lazy.hpp"

namespace rev {
	template <class T, class Maker>
	class SingletonData :
		public std::shared_ptr<T>,
		public SingletonDataLazy<T, Maker>
	{
		private:
			using base_t = SingletonDataLazy<T, Maker>;
			using SP = std::shared_ptr<T>;
		public:
			SingletonData():
				SP(base_t::GetData())
			{}
	};
}
