#pragma once
#include "entry.hpp"
#include "child.hpp"
#include "style.hpp"

namespace rev {
	namespace debug {
		struct StyleBase;
		struct FieldLabel {
			FieldLabel(const char* label);
		};
		class EntryField :
			public StylePush<1>,
			public ChildPush,
			public FieldLabel,
			public Entry
		{
			public:
				EntryField(const char* label, int initial=0, int nLine=0, int columns=2);
		};
	}
}
