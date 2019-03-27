#include "keylog.hpp"
#include "../input/sdlvalue.hpp"

namespace rev {
	KeyLogV SDLKeyLog::s_keylog;
	KeyAux SDLKeyLog::s_keyaux;
	void SDLKeyLog::Update() {
		s_keylog.clear();
		auto lc = g_sdlInputShared.lock();
		lc->procKey([](const int len, const KeyLog& key){
			if(len & 1) {
				s_keylog.emplace_back(key);
			}
		});
		s_keyaux = lc->keyaux;
	}
	const KeyLogV& SDLKeyLog::GetLog() noexcept {
		return s_keylog;
	}
	const KeyAux& SDLKeyLog::GetAux() noexcept {
		return s_keyaux;
	}
}
