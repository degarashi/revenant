#include "../../handle.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			void _Show(Resource& r) {
				r.guiViewer();
			}
			bool _Edit(Resource& r) {
				return r.guiEditor();
			}
		}
	}
}
