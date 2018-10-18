#pragma once
#include <string>

namespace rev {
	class SDLTextInput {
		private:
			static std::string	s_text;
		public:
			static void Update();
			static const std::string& GetText() noexcept;
	};
	using TextInputDep = SDLTextInput;
}
