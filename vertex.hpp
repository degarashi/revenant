#pragma once
#include "spine/enum.hpp"
#include <memory>
#include <vector>

#define SEQ_VSEM (POSITION)(NORMAL)(TEXCOORD)(COLOR)(JOINT)(WEIGHT)(BINORMAL)(TANGENT)

namespace rev {
	DefineEnum(VSem, SEQ_VSEM);
	struct VSemantic {
		VSem	sem;
		int		index;

		bool operator == (const VSemantic& v) const noexcept;
		bool operator < (const VSemantic& v) const noexcept;
	};
	struct VSemAttr {
		VSemantic	sem;
		int			attrId;
	};
	using VSemAttrV = std::vector<VSemAttr>;
}
