#pragma once
#include <memory>
#include <string>
#include "spine/resmgr.hpp"

namespace rev {
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
