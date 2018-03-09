#pragma once
#include "input_common.hpp"

namespace rev {
	using KeyLogV = std::vector<KeyLog>;
	class SDLKeyLog {
		private:
			static KeyLogV	s_keylog;
			static KeyAux	s_keyaux;
		public:
			static void Update();
			static const KeyLogV& GetLog() noexcept;
			static const KeyAux& GetAux() noexcept;
	};
	using KeyLogDep = SDLKeyLog;
}
