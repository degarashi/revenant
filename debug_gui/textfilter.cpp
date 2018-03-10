#include "textfilter.hpp"
#include "state_storage.hpp"
#include "lubee/meta/countof.hpp"
#include <cstring>

namespace rev {
	namespace debug {
		TextFilter::TextFilter(const ImGuiID id):
			ImGuiTextFilter(debug::StateStorage<CBuff256>::GetDefault(id)->data()),
			_buff(*debug::StateStorage<CBuff256>::GetDefault(id))
		{
			if(Draw()) {
				std::strncpy(
					_buff.data(),
					InputBuf,
					std::min<int>(countof(InputBuf), 256)
				);
			}
		}
	}
}
