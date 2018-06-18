#pragma once
#include "../parambase.hpp"

namespace rev::test {
	struct Param : ParamBase {
		HFx makeEffect() const override;
		HScene makeFirstScene() const override;
	};
}
