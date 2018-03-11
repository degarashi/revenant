#pragma once
#include "entry.hpp"
#include "child.hpp"

namespace rev {
	namespace debug {
		class EntryField :
			public ChildPush,
			public Entry
		{
			public:
				EntryField(const char* label, float height=0, int columns=2);
		};
	}
}
