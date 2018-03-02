#pragma once

namespace rev {
	namespace debug {
		class MenuPush {
			private:
				bool	_valid;
			public:
				MenuPush(const char* label, bool enabled=true);
				MenuPush(MenuPush&& m);
				explicit operator bool () const noexcept;
				~MenuPush();
		};
	}
}
