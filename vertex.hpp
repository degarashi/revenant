#pragma once
#include "spine/resmgr.hpp"
#include "spine/enum.hpp"
#include <memory>
#include <string>
#include <boost/preprocessor.hpp>

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

	class VDecl;
	using VDecl_SP = std::shared_ptr<VDecl>;

	template <class>
	class DrawDecl;
}
//! VDeclメソッド定義
#define DefineVDecl(etag) \
	namespace rev { \
		template <> \
		class DrawDecl<etag> { \
			public: \
				static const VDecl_SP& GetVDecl(); \
		}; \
	}
