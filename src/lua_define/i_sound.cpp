#include "../luaimpl.hpp"
#include "../sound.hpp"

DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::ABuffer, ABuffer,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(isStreaming)
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::AGroup, AGroup,
	"ABuffer",
	NOTHING,
	NOTHING,
	(pause)(resume)(clear)(play)(fadeIn)(fadeInOut)(getChannels)(getIdleChannels)(getPlayingChannels)
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::ASource, ASource,
	"ABuffer",
	NOTHING,
	NOTHING,
	(play)(pause)(stop)(setFadeTo)(setFadeOut)(setBuffer)(getLooping)(getNLoop)(setPitch)(setGain)(setRelativeMode)
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::SoundMgr, SoundMgr,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(loadWaveBatch)(loadOggBatch)(loadOggStream)(createSourceGroup)(createSource)
)

namespace rev {
	void LuaImport::RegisterSoundClass(LuaState& lsc) {
		RegisterClass<ABuffer>(lsc);
		RegisterClass<AGroup>(lsc);
		RegisterClass<ASource>(lsc);
		ImportClass(lsc, "System", "sound", &mgr_sound);
	}
}
