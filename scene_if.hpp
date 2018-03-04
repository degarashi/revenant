#pragma once
#include "object.hpp"

namespace rev {
	class IScene : public Object {
		public:
			virtual HGroup getUpdGroup() const;
			virtual HDGroup getDrawGroup() const;
	};
}
