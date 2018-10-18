#pragma once
#include <cstddef>

namespace rev {
	namespace debug {
		struct IdPush {
			bool	_valid;
			IdPush(IdPush&& idp);
			IdPush(std::nullptr_t);
			IdPush(const int id);
			IdPush(const char* id);
			IdPush(const void* id);
			~IdPush();
		};
	}
}
