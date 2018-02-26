#pragma once

namespace rev {
	namespace debug {
		struct IdPush {
			IdPush(const int id);
			IdPush(const char* id);
			IdPush(const void* id);
			~IdPush();
		};
	}
}
