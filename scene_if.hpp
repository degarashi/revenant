#pragma once
#include "object.hpp"

namespace rev {
	class IScene : public IObject {
		public:
			virtual HGroup getUpdGroup() const;
			virtual HDGroup getDrawGroup() const;
	};
}
