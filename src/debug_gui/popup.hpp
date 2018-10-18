#pragma once

using ImGuiWindowFlags = int;
namespace rev {
	namespace debug {
		class PopupPush {
			private:
				bool	_valid;
			public:
				PopupPush(const char* id, ImGuiWindowFlags flags=0);
				PopupPush(PopupPush&& m);
				explicit operator bool () const noexcept;
				~PopupPush();
		};
	}
}
