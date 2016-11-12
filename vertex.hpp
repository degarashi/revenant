#pragma once
#include "spine/resmgr.hpp"
#include "spine/enum.hpp"
#include <memory>
#include <string>
#include <boost/preprocessor.hpp>

#define NUM_TEXCOORD 8
#define PPFUNC_ADDNUM(z,n,data) (BOOST_PP_CAT(data,n))
#define SEQ_VSEM BOOST_PP_REPEAT(NUM_TEXCOORD, PPFUNC_ADDNUM, TEXCOORD)(POSITION)(COLOR)(NORMAL)(BINORMAL)(TANGENT)

namespace rev {
	DefineEnum(VSem, SEQ_VSEM);

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
