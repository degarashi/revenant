#pragma once

namespace rev {
	namespace debug {
		class MenuBarPush {
			private:
				bool	_valid;
			public:
				MenuBarPush();
				MenuBarPush(MenuBarPush&& m);
				explicit operator bool () const noexcept;
				~MenuBarPush();
		};
	}
}
