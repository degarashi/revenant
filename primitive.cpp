#include "primitive.hpp"
#include "lubee/meta/countof.hpp"
#include "drawtoken/glx.hpp"

namespace rev {
	// -------------- Primitive --------------
	bool Primitive::operator != (const Primitive& p) const noexcept {
		return !vertexCmp(p) || !indexCmp(p);
	}
	bool Primitive::vertexCmp(const Primitive& p) const noexcept {
		if(vdecl != p.vdecl)
			return true;
		for(int i=0 ; i<static_cast<int>(countof(vb)) ; i++) {
			if(vb[i] != p.vb[i])
				return true;
		}
		return false;
	}
	bool Primitive::indexCmp(const Primitive& p) const noexcept {
		return ib != p.ib;
	}
	void Primitive::extractVertexData(draw::VStream& dst, const VSemAttrV& vAttrId) const {
		// vertex
		Assert(vdecl, "VDecl is not set");
		dst.spVDecl = vdecl;
		for(int i=0 ; i<static_cast<int>(countof(vb)) ; i++) {
			if(vb[i])
				dst.vbuff[i] = vb[i]->getDrawToken();
		}
		dst.vAttrId = vAttrId;
		if(ib)
			dst.ibuff = draw::Buffer(ib->getDrawToken());
	}
	void Primitive::getArray(CmpArray& dst) const noexcept {
		auto add = [p = dst.data()](auto& ptr) mutable {
			*p++ = reinterpret_cast<uintptr_t>(ptr.get());
		};
		for(auto& v : vb)
			add(v);
		add(ib);
		add(vdecl);
	}
	bool Primitive::operator < (const Primitive& p) const noexcept {
		CmpArray a, b;
		getArray(a);
		p.getArray(b);
		return a < b;
	}
	std::pair<int,int> Primitive::getDifference(const Primitive& p) const noexcept {
		return std::make_pair(
			vertexCmp(p),
			indexCmp(p)
		);
	}
	bool Primitive::hasInfo() const noexcept {
		return vdecl || vb[0] || ib;
	}
}
