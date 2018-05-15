#include "primitive.hpp"
#include "lubee/meta/countof.hpp"
#include "vdecl.hpp"
#include "drawcmd/queue_if.hpp"
#include "vertex_map.hpp"

namespace rev {
	HPrim Primitive::_MakeWithIndex(const FWVDecl& vd, DrawMode mode, const HIb& ib,  const GLsizei count, const GLuint offsetElem) {
		HPrim ret(new Primitive());
		ret->vdecl = vd;
		ret->ib = ib;
		ret->drawMode = mode;
		auto& wi = ret->withIndex;
		wi.count = count;
		wi.offsetElem = offsetElem;
		return ret;
	}
	HPrim Primitive::_MakeWithoutIndex(const FWVDecl& vd, DrawMode mode, const GLint first, const GLsizei count) {
		HPrim ret(new Primitive());
		ret->vdecl = vd;
		ret->drawMode = mode;
		auto& wi = ret->withoutIndex;
		wi.first = first;
		wi.count = count;
		return ret;
	}
	void Primitive::_makeVHash() {
		std::size_t h = lubee::hash_combine_implicit(vdecl);
		lubee::hash_combine_range(h, vb, vb+MaxVStream);
		vhash = h;
	}
	bool Primitive::operator == (const Primitive& p) const noexcept {
		return vertexCmp(p) &&
				indexCmp(p) &&
				drawMode == p.drawMode;
	}
	bool Primitive::operator != (const Primitive& p) const noexcept {
		return !(this->operator == (p));
	}
	bool Primitive::vertexCmp(const Primitive& p) const noexcept {
		if(vhash != p.vhash)
			return false;
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
	void Primitive::_dcmd_export_common(draw::IQueue& q) const {
		if(ib) {
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
	void Primitive::dcmd_export_diff(draw::IQueue& q, const Primitive& prev, const VSemAttrMap& vmap) const {
		if(!vertexCmp(prev)) {
			vdecl->dcmd_export(q, vb, vmap);
		}
		if(ib && !indexCmp(prev)) {
			ib->dcmd_export(q);
		}
		_dcmd_export_common(q);
	}
	void Primitive::dcmd_export(draw::IQueue& q, const VSemAttrMap& vmap) const {
		Assert(vdecl, "VDecl is not set");
		vdecl->dcmd_export(q, vb, vmap);
		if(ib)
			ib->dcmd_export(q);
		_dcmd_export_common(q);
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
	bool Primitive::hasIndex() const noexcept {
		return static_cast<bool>(ib);
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
