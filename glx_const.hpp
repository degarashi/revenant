#pragma once
#include "vertex.hpp"
#include "gl_header.hpp"

namespace rev {
	constexpr static int MaxVStream = 4;
	using VAttrA = GLint[VSem::_Num];
	using VAttrA_CRef = const VAttrA&;
}
