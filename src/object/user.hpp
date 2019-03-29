#pragma once
#include "if.hpp"
#include "id.hpp"

namespace rev {
	class U_Object : public IObject {
		public:
			bool isNode() const noexcept override;
			ObjTypeId getTypeId() const noexcept override;
	};
}
DEF_LUAIMPORT(rev::U_Object)
