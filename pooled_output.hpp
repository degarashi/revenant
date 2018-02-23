#pragma once
#include "rev_output.hpp"
#include <deque>
#include <vector>

namespace rev {
	namespace log {
		//! ログメッセージをバッファに溜めておき、任意のタイミングで取得
		class PooledRevOutput : public RevOutput {
			private:
				using base_t = RevOutput;
				using Type = lubee::log::Type;
				ProcLog		_def[Type::_Num];
				struct Entry {
					Type::e		type;
					std::string	msg;

					Entry(const Type::e typ, const std::string& s):
						type(typ),
						msg(s)
					{}
				};
				// とりあえず素直な実装(1エントリにつき1文字列)
				using EntryD = std::deque<Entry>;
				EntryD			_entry;
				mutable Mutex	_mutex;
				using EntryV = std::vector<Entry>;
				//! 最大エントリ数
				std::size_t		_maxEntry,
				//! バッファに入り切らなくて破棄されたエントリ数
								_discardedEntry;

				void _procMessage(Type::e type, const std::string& msg);
			public:
				PooledRevOutput();
				void setMaxEntry(std::size_t n);
				void clear();
				std::size_t getNDiscardedEntry() const;
				//! 持っているバッファを全て外部へ出力
				EntryV takeOut();
		};
	}
}
