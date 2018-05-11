#include "primitive.hpp"
#include "lubee/meta/countof.hpp"
#include "vdecl.hpp"

namespace rev {
	// -------------- Primitive --------------
	bool Primitive::operator == (const Primitive& p) const noexcept {
		return vertexCmp(p) &&
				indexCmp(p) &&
				drawMode == p.drawMode;
	}
	bool Primitive::operator != (const Primitive& p) const noexcept {
		return !(this->operator == (p));
	}
	bool Primitive::vertexCmp(const Primitive& p) const noexcept {
		if(vdecl != p.vdecl)
			return false;
		for(std::size_t i=0 ; i<countof(vb) ; i++) {
			if(vb[i] != p.vb[i])
				return false;
		}
		return true;
	}
	bool Primitive::indexCmp(const Primitive& p) const noexcept {
		if(ib != p.ib)
			return false;
		if(ib) {
			auto &w0 = withIndex,
				 &w1 = p.withIndex;
			return w0.count == w1.count &&
					w0.offsetElem == w1.offsetElem;
		} else {
			auto &w0 = withoutIndex,
				 &w1 = p.withoutIndex;
			return w0.count == w1.count &&
					w0.first == w1.first;
		}
	}
	void Primitive::dcmd_stream(draw::IQueue& q, const VSemAttrV& vAttr) const {
		Assert(vdecl, "VDecl is not set");
		vdecl->dcmd_apply(q, vb, vAttr);
		if(ib)
			ib->dcmd_use(q);
	}
	void Primitive::dcmd_streamEnd(draw::IQueue& q) const {
		Assert(vdecl, "VDecl is not set");
		for(std::size_t i=0 ; i<countof(vb) ; i++) {
			if(vb[i]) {
				vb[i]->dcmd_useEnd(q);
				break;
			}
		}
		if(ib)
			ib->dcmd_useEnd(q);
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
			!vertexCmp(p),
			!indexCmp(p)
		);
	}
	bool Primitive::hasInfo() const noexcept {
		return vdecl || vb[0] || ib;
	}
}
