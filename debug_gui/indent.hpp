#pragma once

namespace rev {
	namespace debug {
		struct IndentPush {
			IndentPush(float w=0.f);
			~IndentPush();
		};
	}
}
