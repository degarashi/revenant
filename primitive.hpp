#pragma once
#include "glx_const.hpp"
#include "gl_types.hpp"
#include "debuggui_if.hpp"
#include "handle/opengl.hpp"

namespace rev {
	namespace draw {
		class IQueue;
	}
	struct VSem_AttrId;
	using VSemAttrV = std::vector<VSem_AttrId>;
	struct Primitive : IDebugGui {
		HVDecl		vdecl;
		HVb			vb[MaxVStream];
		HIb			ib;
		DrawMode	drawMode;
		union {
			struct {
				// 描画に使用される要素数
				GLsizei		count;
				// オフセット要素数
				GLuint		offsetElem;
			} withIndex;
			struct {
				// 描画を開始する要素オフセット
				GLint		first;
				// 描画に使用される要素数
				GLsizei		count;
			} withoutIndex;
		};
		constexpr static int NArray = MaxVStream + 2;
		using CmpArray = std::array<uintptr_t, NArray>;

		bool vertexCmp(const Primitive& p) const noexcept;
		bool indexCmp(const Primitive& p) const noexcept;
		std::pair<int,int> getDifference(const Primitive& p) const noexcept;
		void dcmd_stream(draw::IQueue& q, const VSemAttrV& vAttr) const;
		void dcmd_streamEnd(draw::IQueue& q) const;
		void getArray(CmpArray& dst) const noexcept;
		bool operator != (const Primitive& p) const noexcept;
		bool operator < (const Primitive& p) const noexcept;
		bool hasInfo() const noexcept;
		DEF_DEBUGGUI_PROP
	};
}
