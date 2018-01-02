#pragma once
#include "handle.hpp"
#include "glx_const.hpp"

namespace rev {
	namespace draw {
		class VStream;
	}
	struct Primitive {
		VDecl_SP	vdecl;
		HVb			vb[MaxVStream];
		HIb			ib;

		bool vertexCmp(const Primitive& p) const noexcept;
		bool indexCmp(const Primitive& p) const noexcept;
		std::pair<int,int> getDifference(const Primitive& p) const noexcept;
		void extractData(draw::VStream& dst, const VSemAttrV& vAttr) const;
		bool operator != (const Primitive& p) const noexcept;
		void reset();
	};
}
