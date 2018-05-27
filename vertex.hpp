#pragma once
#include "spine/enum.hpp"
#include "lubee/hash_combine.hpp"

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
		bool		bInteger;

		bool operator == (const VSem_AttrId& a) const noexcept;
		bool operator < (const VSem_AttrId& a) const noexcept;
	};
}
namespace std {
	template <>
	struct hash<rev::VSemantic> {
		std::size_t operator()(const rev::VSemantic& m) const noexcept {
			return lubee::hash_combine_implicit(m.sem.value, m.index);
		}
	};
	template <>
	struct hash<rev::VSem_AttrId> {
		std::size_t operator()(const rev::VSem_AttrId& m) const noexcept {
			return lubee::hash_combine_implicit(m.sem, m.attrId, m.bInteger);
		}
	};
}
