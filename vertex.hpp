#pragma once
#include "spine/enum.hpp"

#define SEQ_VSEM (POSITION)(NORMAL)(TEXCOORD)(COLOR)(JOINT)(WEIGHT)(BINORMAL)(TANGENT)

namespace rev {
	DefineEnum(VSemEnum, SEQ_VSEM);
	struct VSemantic {
		VSemEnum	sem;
		int			index;

		bool operator == (const VSemantic& v) const noexcept;
		bool operator < (const VSemantic& v) const noexcept;
	};
	// SemanticからシェーダのAttributeIdへの対応付け
	struct VSem_AttrId {
		VSemantic	sem;
		int			attrId;
	};
}
