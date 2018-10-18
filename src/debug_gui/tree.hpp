#pragma once

using ImGuiTreeNodeFlags = int;
namespace rev {
	namespace debug {
		class TreePush {
			private:
				bool _valid;
			public:
				TreePush(const char* label, ImGuiTreeNodeFlags flag=0);
				TreePush(TreePush&& t);
				~TreePush();
				explicit operator bool () const noexcept;
		};
	}
}
