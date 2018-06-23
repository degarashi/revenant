#pragma once
#include "../../sdl_tls.hpp"
#include "../../handle/input.hpp"
#include "spine/enum.hpp"

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
