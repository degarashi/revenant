#pragma once
#include <string>
#include <memory>

using ImGuiTreeNodeFlags = int;
namespace rev::debug {
	class TreePush;
	class Header {
		private:
			using Tree_UP = std::shared_ptr<TreePush>;
			Tree_UP	_tree;
			static ImGuiTreeNodeFlags _GetFlag(bool headerOnly, bool defaultOpen);
			inline static std::string	s_label;
			static const char* _GetLabel(const char* label, bool headerOnly);
		public:
			Header(const char* label, bool headerOnly, bool defaultOpen=false, bool hideWhenEmpty=false);
			explicit operator bool () const noexcept;
	};
}
