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
		constexpr static int NArray = MaxVStream + 2;
		using CmpArray = std::array<uintptr_t, NArray>;

		bool vertexCmp(const Primitive& p) const noexcept;
		bool indexCmp(const Primitive& p) const noexcept;
		std::pair<int,int> getDifference(const Primitive& p) const noexcept;
		void extractData(draw::VStream& dst, const VSemAttrV& vAttr) const;
		void getArray(CmpArray& dst) const noexcept;
		bool operator != (const Primitive& p) const noexcept;
		bool operator < (const Primitive& p) const noexcept;
		void reset();
		bool hasInfo() const noexcept;
	};
	using Primitive_SP = std::shared_ptr<Primitive>;
	using Primitive_WP = std::weak_ptr<Primitive>;
}
