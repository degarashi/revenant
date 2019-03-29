#include "if.hpp"

namespace rev {
	HGroup IScene::getUpdGroup() const {
		D_Expect(false, "invalid function call");
		return HGroup();
	}
	HDGroup IScene::getDrawGroup() const {
		D_Expect(false, "invalid function call");
		return HDGroup();
	}
}
