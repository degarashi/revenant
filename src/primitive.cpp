#include "primitive.hpp"
#include "lubee/src/meta/countof.hpp"
#include "vdecl.hpp"
#include "drawcmd/queue_if.hpp"
#include "vertex_map.hpp"

namespace rev {
	Primitive::Primitive():
		cache([this](const FWVMap& vm){
			draw::CommandVec cmd;
			vdecl->dcmd_export(cmd, vb, vm);
			return cmd;
		})
	{}
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
		lubee::hash_combine_range(h, vb.begin(), vb.end());
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
		return vb == p.vb;
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
	void Primitive::dcmd_export_diff(draw::IQueue& q, const Primitive& prev, const FWVMap& vmap) const {
		cache.clear();
		if(!vertexCmp(prev)) {
			q.append(cache.getCache(vmap));
		}
		if(ib && !indexCmp(prev)) {
			ib->dcmd_export(q);
		}
		_dcmd_export_common(q);
	}
	void Primitive::dcmd_export(draw::IQueue& q, const FWVMap& vmap) const {
		cache.clear();
		Assert(vdecl, "VDecl is not set");
		q.append(cache.getCache(vmap));
		if(ib)
			ib->dcmd_export(q);
		_dcmd_export_common(q);
	}
	bool Primitive::operator < (const Primitive& p) const noexcept {
		const std::array<uintptr_t, 3>	ar[2] = {
			{
				reinterpret_cast<uintptr_t>(vdecl.get()),
				reinterpret_cast<uintptr_t>(ib.get()),
				static_cast<uintptr_t>(drawMode)
			},
			{
				reinterpret_cast<uintptr_t>(p.vdecl.get()),
				reinterpret_cast<uintptr_t>(p.ib.get()),
				static_cast<uintptr_t>(p.drawMode)
			}
		};
		if(std::lexicographical_compare(
				ar[0].begin(), ar[0].end(),
				ar[1].begin(), ar[1].end()))
			return true;
		return std::lexicographical_compare(
			vb.begin(), vb.end(),
			p.vb.begin(), p.vb.end());
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
	const FWVDecl& Primitive::getVDecl() const noexcept {
		return vdecl;
	}
}
#include "gl/if.hpp"
#include "gl/error.hpp"
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
