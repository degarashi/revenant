#pragma once
#include "object_if.hpp"
#include "object_id.hpp"

namespace rev {
	class U_Object : public Object, public ObjectIdT<detail::IdDummy, idtag::Object> {
		private:
			using IdT = ObjectIdT<detail::IdDummy, idtag::Object>;
		public:
			bool isNode() const noexcept override;
			ObjTypeId getTypeId() const noexcept override;
	};
}
DEF_LUAIMPORT(rev::U_Object)
