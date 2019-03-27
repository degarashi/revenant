#include "shared.hpp"
#include "../../input/input.hpp"

namespace rev::test {
	TLS<UserShare>	tls_shared;
	UserShare::UserShare() {
		const auto hKb = Keyboard::OpenKeyboard();
		const auto hM = Mouse::OpenMouse(0);
		act[Act::CMoveX] = mgr_input.makeAction("move_x");
		act[Act::CMoveX]->linkButtonAsAxis(hKb, VKey::A, VKey::D);
		act[Act::CMoveY] = mgr_input.makeAction("move_y");
		act[Act::CMoveY]->linkButtonAsAxis(hKb, VKey::Lshift, VKey::Space);
		act[Act::CMoveZ] = mgr_input.makeAction("move_z");
		act[Act::CMoveZ]->linkButtonAsAxis(hKb, VKey::S, VKey::W);
		act[Act::CDirX] = mgr_input.makeAction("dir_x");
		act[Act::CDirX]->addLink(hM, InputFlag::Axis, 0);
		act[Act::CDirY] = mgr_input.makeAction("dir_y");
		act[Act::CDirY]->addLink(hM, InputFlag::Axis, 1);
		act[Act::CDirBtn] = mgr_input.makeAction("dir_b");
		act[Act::CDirBtn]->addLink(hM, InputFlag::Button, 0);
	}
}
