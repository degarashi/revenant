#pragma once

namespace rev {
	namespace debug {
		class MainMenuBarPush {
			private:
				bool	_valid;
			public:
				MainMenuBarPush();
				MainMenuBarPush(MainMenuBarPush&& m);
				explicit operator bool () const noexcept;
				~MainMenuBarPush();
		};
	}
}
