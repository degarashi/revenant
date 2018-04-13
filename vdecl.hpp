#pragma once
#include "vertex.hpp"
#include "spine/optional.hpp"
#include "gl_buffer.hpp"
#include "glx_const.hpp"

namespace rev {
	using VSem_AttrV = std::vector<VSem_AttrId>;
	//! DirectX9ライクな頂点宣言
	class VDecl {
		private:
			template <class Ar>
			friend void save(Ar&, const VDecl&);
			template <class Ar>
			friend void load(Ar&, const VDecl&);
			#ifdef DEBUGGUI_ENABLED
				friend struct Primitive;
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
			using Setter = std::function<void (GLuint, const VSem_AttrV&)>;
			using SetterV = std::vector<Setter>;
			// ストリーム毎のサイズを1次元配列で格納 = 0番から並べる
			SetterV		_setter;
			// 各ストリームの先頭インデックス
			std::size_t	_streamOfs[MaxVStream+1];
			// 元データ(シリアライズ用)
			VDInfoV		_vdInfo;

			static VDInfoV _ToVector(std::initializer_list<VDInfo>& il);
			void _init();

		public:
			VDecl() = default;
			VDecl(const VDInfoV& vl);
			//! 入力: {streamId, offset, GLFlag, bNoramalize, semantics}
			VDecl(std::initializer_list<VDInfo> il);
			//! OpenGLへ頂点位置を設定
			// [描画スレッドからの呼び出し]
			void apply(const GLBufferCore* (&stream)[MaxVStream], const VSem_AttrV& attr) const;
			bool operator == (const VDecl& vd) const;
			bool operator != (const VDecl& vd) const;
			#ifdef DEBUGGUI_ENABLED
				using CBProp = std::function<void (const VDInfo&)>;
				void property(const CBProp& cb) const;
			#endif
	};
}
