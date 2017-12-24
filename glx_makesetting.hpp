#pragma once
#include <memory>

namespace rev {
	namespace parse {
		struct ValueSetting;
		struct BoolSetting;
	}
	struct GLState;
	using GLState_SP = std::shared_ptr<GLState>;
	GLState_SP MakeValueSetting(const parse::ValueSetting& s);
	GLState_SP MakeBoolSetting(const parse::BoolSetting& s);
}
