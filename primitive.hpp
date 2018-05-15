#pragma once
#include "glx_const.hpp"
#include "gl_types.hpp"
#include "debuggui_if.hpp"
#include "handle/opengl.hpp"
#include "cache.hpp"
#include "drawcmd/cmd.hpp"

namespace rev {
	namespace draw {
		class IQueue;
	}
	class Primitive : public IDebugGui {
		private:
			friend struct std::hash<Primitive>;
			//! Draw token (without index)
			struct DCmd_Draw {
				GLenum		mode;
				GLint		first;
				GLsizei		count;

				static void Command(const void* p);
			};
			//! Draw token (with index)
			struct DCmd_DrawIndexed {
				//! 描画モードフラグ(OpenGL)
				GLenum			mode;
				//! 描画に使用される要素数
				GLsizei			count;
				//! 1要素のサイズを表すフラグ
				GLenum			sizeF;
				//! オフセットバイト数
				GLuint			offset;

				static void Command(const void* p);
			};

			FWVDecl		vdecl;
			HVb			vb[MaxVStream];
			std::size_t	vhash;
			HIb			ib;
			DrawMode	drawMode;

			using Cache_t = Cache<FWVMap, draw::CommandVec>;
			mutable Cache_t	cache;

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

			static void _SetVB(HVb*) {}
			template <class... VBs>
			static void _SetVB(HVb* dst, const HVb& vb, const VBs&... vbs) {
				*dst = vb;
				_SetVB(dst+1, vbs...);
			}
			static void _SetVB(HVb* dst, const HVb* vb, std::size_t n) {
				while(n-- != 0) {
					*dst++ = *vb++;
				}
			}
			static HPrim _MakeWithIndex(const FWVDecl& vd, DrawMode mode, const HIb& ib,  const GLsizei count, const GLuint offsetElem);
			static HPrim _MakeWithoutIndex(const FWVDecl& vd, DrawMode mode, const GLint first, const GLsizei count);
			Primitive();
			void _dcmd_export_common(draw::IQueue& q) const;
			void _makeVHash();

		public:
			template <class... VBs>
			static HPrim MakeWithIndex(const FWVDecl& vd, DrawMode mode, const HIb& ib,  const GLsizei count, const GLuint offsetElem, const VBs&... vbs) {
				HPrim ret = _MakeWithIndex(vd, mode, ib, count, offsetElem);
				_SetVB(ret->vb, vbs...);
				ret->_makeVHash();
				return ret;
			}
			template <class... VBs>
			static HPrim MakeWithoutIndex(const FWVDecl& vd, DrawMode mode, const GLint first, const GLsizei count, const VBs&... vbs) {
				HPrim ret = _MakeWithoutIndex(vd, mode, first, count);
				_SetVB(ret->vb, vbs...);
				ret->_makeVHash();
				return ret;
			}
			bool vertexCmp(const Primitive& p) const noexcept;
			bool indexCmp(const Primitive& p) const noexcept;
			std::pair<int,int> getDifference(const Primitive& p) const noexcept;
			void dcmd_export(draw::IQueue& q, const FWVMap& vmap) const;
			void dcmd_export_diff(draw::IQueue& q, const Primitive& prev, const FWVMap& vmap) const;
			void getArray(CmpArray& dst) const noexcept;
			bool operator == (const Primitive& p) const noexcept;
			bool operator != (const Primitive& p) const noexcept;
			bool operator < (const Primitive& p) const noexcept;
			bool hasIndex() const noexcept;
			DEF_DEBUGGUI_PROP
	};
}
#include "lubee/hash_combine.hpp"
namespace std {
	template <>
	struct hash<rev::Primitive> {
		std::size_t operator()(const rev::Primitive& p) const noexcept {
			std::size_t ret = lubee::hash_combine_implicit(p.vdecl);
			lubee::hash_combine_range(ret, p.vb, p.vb+rev::MaxVStream);
			lubee::hash_combine(ret, p.ib, p.drawMode.value);
			return ret;
		}
	};
}
