#pragma once

using ImGuiWindowFlags = int;
struct ImVec2;
namespace rev {
	namespace debug {
		class ChildPush {
			private:
				bool _result,
					 _destroy;
			public:
				ChildPush(const char* id, const ImVec2& size, bool border=false, ImGuiWindowFlags flags=0);
				ChildPush(const char* id, float height, bool border=false, ImGuiWindowFlags flags=0);
				ChildPush(ChildPush&& c);
				~ChildPush();
				void close();
				explicit operator bool () const noexcept;
		};
	}
}
