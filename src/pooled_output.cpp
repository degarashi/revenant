#include "pooled_output.hpp"

namespace rev {
	namespace {
		constexpr std::size_t DefaultMaxEntry = 256;
	}
	namespace log {
		PooledRevOutput::PooledRevOutput():
			_maxEntry(DefaultMaxEntry),
			_discardedEntry(0)
		{
			for(int i=0 ; i<Type::_Num ; i++) {
				_def[i] = _proc[i];
				const auto typ = static_cast<Type::e>(i);
				_proc[typ] = [this, typ](const auto& msg){ _procMessage(typ, msg); };
			}
		}
		void PooledRevOutput::_procMessage(const Type::e type, const std::string& msg) {
			const UniLock lk(_mutex);
			if(_entry.size() == _maxEntry) {
				// バッファに収まり切らないので古いエントリを削除
				_entry.pop_front();
				++_discardedEntry;
			}
			_entry.emplace_back(type, msg);
			_def[type](msg);
		}
		void PooledRevOutput::clear() {
			const UniLock lk(_mutex);
			_entry.clear();
			_discardedEntry = 0;
		}
		void PooledRevOutput::setMaxEntry(const std::size_t n) {
			const UniLock lk(_mutex);
			_maxEntry = n;
			const std::size_t cur = _entry.size();
			if(cur <= n)
				return;
			// バッファから溢れる分は削除
			_entry.erase(_entry.begin(), _entry.begin()+(cur-n));
		}
		std::size_t PooledRevOutput::getNDiscardedEntry() const {
			const UniLock lk(_mutex);
			return _discardedEntry;
		}
		PooledRevOutput::EntryV PooledRevOutput::takeOut() {
			const UniLock lk(_mutex);
			EntryV ret;
			std::move(_entry.begin(), _entry.end(), std::back_inserter(ret));
			clear();
			return ret;
		}
	}
}
