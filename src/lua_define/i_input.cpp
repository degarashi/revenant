#include "../lua/impl.hpp"
#include "../input/input.hpp"

DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::Input, Input,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(name)(getButton)(getAxis)(getHat)(numButtons)(numAxes)(numHats)
	(setDeadZone)(setMouseMode)(getMouseMode)(getPointer)
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::Action, Action,
	LUAIMPLEMENT_BASE,
	NOTHING,
	NOTHING,
	(isKeyPressed)(isKeyReleased)(isKeyPressing)(addLink)(remLink)(getState)(getValue)
	(getKeyValueSimplified)(linkButtonAsAxis)
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::Keyboard, Keyboard,
	"Input",
	(OpenKeyboard),
	NOTHING,
	NOTHING
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::Mouse, Mouse,
	"Input",
	(OpenMouse)(NumMouse),
	NOTHING,
	NOTHING
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::Joypad, Joypad,
	"Input",
	(OpenJoypad)(NumJoypad),
	NOTHING,
	NOTHING
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	TPos2D, TPos2D,
	LUAIMPLEMENT_BASE,
	NOTHING,
	(absPos)(relPos)(pressure),
	(setNewAbs)(setNewRel)
)
DEF_LUAIMPLEMENT_PTR_NOCTOR(
	rev::InputMgr, InputMgr,
	LUAIMPLEMENT_BASE,
	(LinkButtonAsAxis),
	NOTHING,
	(makeAction)
)

namespace rev {
	void LuaImport::RegisterInputClass(LuaState& lsc) {
		RegisterClass<Input>(lsc);
		RegisterClass<Action>(lsc);
		RegisterClass<Keyboard>(lsc);
		RegisterClass<Mouse>(lsc);
		RegisterClass<Joypad>(lsc);
		RegisterClass<TPos2D>(lsc);
		ImportClass(lsc, "System", "input", &mgr_input);
	}
}
