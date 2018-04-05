#include "textfilter.hpp"
#include "lubee/meta/countof.hpp"
#include <cstring>

namespace rev::debug {
	namespace {
		constexpr std::size_t BufferLen = 256;
	}

	TextFilter::TextFilter(const ImGuiID id):
		TextBuffer(id, BufferLen),
		ImGuiTextFilter(buffer().data())
	{
		if(Draw()) {
			std::strncpy(
				buffer().data(),
				InputBuf,
				std::min<int>(countof(InputBuf), BufferLen)
			);
		}
	}
}
