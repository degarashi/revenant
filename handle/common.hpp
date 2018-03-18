#pragma once
#include <memory>

#define DEF_HANDLE(typ, name) \
	using H##name = std::shared_ptr<typ>; \
	using W##name = std::weak_ptr<typ>;
