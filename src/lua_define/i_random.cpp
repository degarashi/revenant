#include "../lua/impl.hpp"
#include "lubee/src/random.hpp"

DEF_LUAIMPLEMENT_PTR_NOCTOR(
	lubee::RandomMT, RandomMT,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(luaGetUniform<float>)(luaGetUniform<int>)
)

namespace rev {
	void LuaImport::RegisterRandomClass(LuaState& lsc) {
		RegisterClass<lubee::RandomMT>(lsc);
	}
}
