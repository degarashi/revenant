#pragma once
#include "entry.hpp"

namespace rev {
	namespace debug {
		class EntryField :
			public IdPush,
			public Entry
		{
			private:
				EntryField(IdPush&& idp, int columns);
			public:
				template <class Id>
				EntryField(const Id& id, int columns=2):
					EntryField(IdPush(id), columns)
				{}
		};
	}
}
