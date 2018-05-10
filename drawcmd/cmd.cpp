#include "cmd.hpp"
#include "lubee/error.hpp"
#include <cstring>

namespace rev::draw {
	namespace {
		struct Cmd {
			std::size_t	size;
			CommandF	func;
			uint8_t		payload[];
		};
	}
	void* CommandVec::_allocateMemory(const std::size_t s, const CommandF f) {
		constexpr auto S = sizeof(std::size_t),
						F = sizeof(CommandF);
		// [次へのオフセット(std::size_t)][処理関数(CommandF)][コマンドのデータ]
		const auto prevLen = _vec.size(),
						len = prevLen + s + S + F;
		_vec.resize(len);

		uint8_t* ptr = _vec.data() + prevLen;
		auto* cmd = reinterpret_cast<Cmd*>(ptr);
		cmd->size = s + S + F;
		cmd->func = f;
		return cmd->payload;
	}
	void CommandVec::exec() {
		const uint8_t *ptr = _vec.data(),
					*end = ptr + _vec.size();
		while(ptr < end) {
			auto* cmd = reinterpret_cast<const Cmd*>(ptr);
			cmd->func(cmd->payload);
			ptr += cmd->size;
		}
		D_Assert0(ptr == end);
	}
	void CommandVec::clear() {
		_vec.clear();
		_set.clear();
	}
	void CommandVec::stockResource(const VoidC_SP& r) {
		_set.emplace(r);
	}
	void CommandVec::getResource(const CBResource& cb) const {
		for(auto& r : _set)
			cb(r);
	}
	IQueue::DataP CommandVec::getData() const {
		return {_vec.data(), _vec.size()};
	}
	void CommandVec::_copyMemory(const DataP& src) {
		const auto prevLen = _vec.size();
		_vec.resize(prevLen + src.second);
		std::memcpy(_vec.data() + prevLen, src.first, src.second);
	}
}
