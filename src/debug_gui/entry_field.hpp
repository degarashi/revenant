#pragma once
#include "entry.hpp"

namespace rev {
	namespace debug {
		class EntryField :
			public IdPush,
			public Entry
		{
			private:
				EntryField(IdPush&& idp, bool edit, int columns);
			public:
				template <class Id>
				EntryField(const Id& id, const bool edit, int columns=2):
					EntryField(IdPush(id), edit, columns)
				{}
		};
	}
}
