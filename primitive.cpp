#include "primitive.hpp"
#include "lubee/meta/countof.hpp"
#include "vdecl.hpp"
#include "drawcmd/queue_if.hpp"

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
	void Primitive::dcmd_export(draw::IQueue& q, const VSemAttrV& vAttr) const {
		Assert(vdecl, "VDecl is not set");
		vdecl->dcmd_export(q, vb, vAttr);
		if(ib) {
			ib->dcmd_export(q);
			const auto str = ib->getStride();
			const auto szF = GLIBuffer::GetSizeFlag(str);
			q.add(DCmd_DrawIndexed{
				.mode = drawMode,
				.count = withIndex.count,
				.sizeF = szF,
				.offset = withIndex.offsetElem*str,
			});
		} else {
			q.add(DCmd_Draw{
				.mode = drawMode,
				.first = withoutIndex.first,
				.count = withoutIndex.count,
			});
		}
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
#include "gl_if.hpp"
#include "gl_error.hpp"
namespace rev {
	void Primitive::DCmd_Draw::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Draw*>(p);
		GL.glDrawArrays(self.mode, self.first, self.count);
		D_GLAssert0();
	}
	void Primitive::DCmd_DrawIndexed::Command(const void* p) {
		auto& self = *static_cast<const DCmd_DrawIndexed*>(p);
		GL.glDrawElements(self.mode, self.count, self.sizeF, reinterpret_cast<const GLvoid*>(self.offset));
		D_GLAssert0();
	}
}
