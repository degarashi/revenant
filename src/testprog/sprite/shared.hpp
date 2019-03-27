#pragma once
#include "../../sdl/tls.hpp"
#include "../../handle/input.hpp"
#include "spine/src/enum.hpp"

namespace rev::test {
	struct UserShare {
		DefineEnum(Act,
			(CMoveX)(CMoveY)(CMoveZ)
			(CDirX)(CDirY)(CDirBtn)
		);
		HAct	act[Act::_Num];

		UserShare();
	};
	extern TLS<UserShare>	tls_shared;
}
