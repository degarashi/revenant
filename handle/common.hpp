#pragma once
#include <memory>

#define DEF_HANDLE(typ, name) \
	using H##name = std::shared_ptr<typ>; \
	using H##name##C = std::shared_ptr<std::add_const_t<typ>>; \
	using W##name = std::weak_ptr<typ>;
