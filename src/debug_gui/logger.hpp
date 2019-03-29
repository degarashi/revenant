#pragma once
#include "../log/logentry.hpp"
#include <memory>
#include <vector>

namespace rev {
	class Logger {
		private:
			using Type = lubee::log::Type;
			using EntryV = std::vector<log::Entry>;
			EntryV _entry;
			mutable struct View {
				char	filter[128];
				struct {
					bool	type[Type::_Num];
				} show;
				bool scrBottom;

				View();
			} _view;
			mutable bool		_clearFlag,
								_filterFlag;
			mutable int			_selected;
			// フィルタ済みのログリスト(インデックス)
			using FilteredIndexV = std::vector<int>;
			FilteredIndexV _filteredIndex;

			void _filter();
		public:
			Logger();
			void drawGUI(bool* p_open=nullptr) const;
			void update();
			void clear();
	};
}
