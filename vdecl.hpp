#pragma once
#include "vertex.hpp"
#include "spine/optional.hpp"
#include "gl_buffer.hpp"
#include "glx_const.hpp"
#include "handle/opengl.hpp"

namespace rev {
	//! DirectX9ライクな頂点宣言
	class VDecl {
		private:
			template <class Ar>
			friend void save(Ar&, const VDecl&);
			template <class Ar>
			friend void load(Ar&, const VDecl&);
			#ifdef DEBUGGUI_ENABLED
				friend class Primitive;
			#endif
		public:
			struct VDInfo {
				template <class Ar>
				friend void serialize(Ar&, VDInfo&);
				GLuint		streamId,		//!< 便宜上の)ストリームId
							offset,			//!< バイトオフセット
							elemFlag,		//!< OpenGLの要素フラグ
							bNormalize,		//!< OpenGLが正規化するか(bool)
							elemSize;		//!< 要素数
				VSemantic	sem;
				GLuint		strideOvr;		//!< 頂点サイズ(上書き) optional

				VDInfo() = default;
				VDInfo(GLuint streamId, GLuint offset, GLuint elemFlag,
						GLuint bNormalize, GLuint elemSize,
						VSemantic sem, GLuint strideOvr=0);
				bool operator == (const VDInfo& v) const;
				bool operator != (const VDInfo& v) const;
				#ifdef DEBUGGUI_ENABLED
					void showAsRow() const;
				#endif
			};
			using VDInfoV = std::vector<VDInfo>;
		private:
			using Setter = std::function<void (draw::IQueue&, GLuint, const VSemAttrMap&)>;
			using SetterV = std::vector<Setter>;
			// ストリーム毎のサイズを1次元配列で格納 = 0番から並べる
			SetterV		_setter;
			// 各ストリームの先頭インデックス
			std::size_t	_streamOfs[MaxVStream+1];
			// 元データ(シリアライズ用)
			VDInfoV		_vdInfo;
			friend struct std::hash<rev::VDecl>;

			struct DCmd_VPtr {
				int			attrId;
				GLuint		elemSize,
							elemFlag,
							stride,
							bNormalize;
				const void*	offset;
				bool		bInteger;

				static void Command(const void* p);
			};

			static VDInfoV _ToVector(std::initializer_list<VDInfo>& il);
			void _init();

		public:
			VDecl() = default;
			VDecl(const VDInfoV& vl);
			//! 入力: {streamId, offset, GLFlag, bNoramalize, semantics}
			VDecl(std::initializer_list<VDInfo> il);
			//! OpenGLへ頂点位置を設定
			void dcmd_export(draw::IQueue& q, const HVb (&stream)[MaxVStream], const VSemAttrMap& vmap) const;
			bool operator == (const VDecl& vd) const;
			bool operator != (const VDecl& vd) const;
			#ifdef DEBUGGUI_ENABLED
				using CBProp = std::function<void (const VDInfo&)>;
				void property(const CBProp& cb) const;
			#endif
	};
}
#include "lubee/hash_combine.hpp"
namespace std {
	template <>
	struct hash<rev::VDecl::VDInfo> {
		std::size_t operator()(const rev::VDecl::VDInfo& vd) const noexcept {
			return lubee::hash_combine_implicit(
				vd.streamId,
				vd.offset,
				vd.elemFlag,
				vd.bNormalize,
				vd.elemSize,
				vd.sem.sem.value,
				vd.sem.index,
				vd.strideOvr
			);
		}
	};
	template <>
	struct hash<rev::VDecl> {
		std::size_t operator()(const rev::VDecl& vd) const noexcept {
			auto& vdi = vd._vdInfo;
			return lubee::hash_combine_range_implicit(vdi.begin(), vdi.end());
		}
	};
}
