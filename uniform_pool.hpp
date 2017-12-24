#pragma once
#include "spine/object_pool.hpp"
#include "drawtoken/tokenbuffer.hpp"
#include "spine/singleton.hpp"

namespace rev {
	#define unif_pool (::rev::UnifPool::ref())
	struct UnifPool :
		spi::ObjectPool<draw::TokenBuffer>,
		spi::Singleton<UnifPool>
	{
		using spi::ObjectPool<draw::TokenBuffer>::ObjectPool;
	};
}
