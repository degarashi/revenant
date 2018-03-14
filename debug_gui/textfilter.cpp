#include "textfilter.hpp"
#include "state_storage.hpp"
#include "lubee/meta/countof.hpp"
#include <cstring>

namespace rev {
	namespace debug {
		TextFilter::TextFilter(const ImGuiID id):
			ImGuiTextFilter(St::GetDefault(id)->data()),
			_buff(*St::GetDefault(id))
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
