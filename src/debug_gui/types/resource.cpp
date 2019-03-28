#include "../if.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			void _Show(IDebugGui& g) {
				g.property(false);
			}
			bool _Edit(IDebugGui& g) {
				return g.property(true);
			}
		}
	}
}
