#include "../../handle.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			bool _Edit(Resource& r) {
				return r.guiEditor();
			}
		}
	}
}
