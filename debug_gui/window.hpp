#pragma once

using ImGuiWindowFlags = int;
struct ImVec2;
namespace rev {
	namespace debug {
		class WindowPush {
			private:
				bool _result,
					 _destroy;
			public:
				WindowPush(const char* label, bool* p_open=nullptr, ImGuiWindowFlags flags=0);
				WindowPush(const char* label, bool* p_open, const ImVec2& size_on_first_use, float bg_alpha_override=-1.f, ImGuiWindowFlags flags=0);
				WindowPush(WindowPush&& w);
				~WindowPush();
				explicit operator bool () const noexcept;
		};
	}
}
