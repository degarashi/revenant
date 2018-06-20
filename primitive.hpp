#pragma once
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
			using HVbV = std::vector<HVb>;

			FWVDecl		vdecl;
			HVbV		vb;
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

			void _setVB() {}
			template <class... VBs>
			void _setVB(const HVb& vb, const VBs&... vbs) {
				this->vb.emplace_back(vb);
				_setVB(vbs...);
			}
			void _setVB(const HVb* vb, const std::size_t n) {
				this->vb.assign(vb, vb+n);
			}
			static HPrim _MakeWithIndex(const FWVDecl& vd, DrawMode mode, const HIb& ib,  const GLsizei count, const GLuint offsetElem);
			static HPrim _MakeWithoutIndex(const FWVDecl& vd, DrawMode mode, const GLint first, const GLsizei count);
			Primitive();
			void _dcmd_export_common(draw::IQueue& q) const;
			void _makeVHash();

		public:
			const FWVDecl& getVDecl() const noexcept;
			template <class... VBs>
			static HPrim MakeWithIndex(const FWVDecl& vd, DrawMode mode, const HIb& ib,  const GLsizei count, const GLuint offsetElem, const VBs&... vbs) {
				HPrim ret = _MakeWithIndex(vd, mode, ib, count, offsetElem);
				ret->_setVB(vbs...);
				ret->_makeVHash();
				return ret;
			}
			template <class... VBs>
			static HPrim MakeWithoutIndex(const FWVDecl& vd, DrawMode mode, const GLint first, const GLsizei count, const VBs&... vbs) {
				HPrim ret = _MakeWithoutIndex(vd, mode, first, count);
				ret->_setVB(vbs...);
				ret->_makeVHash();
				return ret;
			}
			bool vertexCmp(const Primitive& p) const noexcept;
			bool indexCmp(const Primitive& p) const noexcept;
			std::pair<int,int> getDifference(const Primitive& p) const noexcept;
			void dcmd_export(draw::IQueue& q, const FWVMap& vmap) const;
			void dcmd_export_diff(draw::IQueue& q, const Primitive& prev, const FWVMap& vmap) const;
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
			lubee::hash_combine_range(ret, p.vb.begin(), p.vb.end());
			lubee::hash_combine(ret, p.ib, p.drawMode.value);
			return ret;
		}
	};
}
