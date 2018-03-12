#include "../../handle.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			void _Show(Resource& r) {
				r.guiEditor(false);
			}
			bool _Edit(Resource& r) {
				return r.guiEditor(true);
			}
		}
	}
}
