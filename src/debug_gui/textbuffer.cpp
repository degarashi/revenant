#include "textbuffer.hpp"
#include "state_storage.hpp"

namespace rev::debug {
	TextBuffer::TextBuffer(const ImGuiID id, const std::size_t size):
		_buff(*St::GetWithMake(id, [size](){
			auto ret = std::make_shared<CBuff>();
			ret->resize(size);
			return ret;
		}))
	{}
	const TextBuffer::CBuff& TextBuffer::buffer() const noexcept {
		return _buff;
	}
	const char* TextBuffer::pointer() const noexcept {
		return _buff.data();
	}
	TextBuffer::CBuff& TextBuffer::buffer() noexcept {
		return _buff;
	}
	char* TextBuffer::pointer() noexcept {
		return _buff.data();
	}
	std::size_t TextBuffer::size() const noexcept {
		return _buff.size();
	}
}
