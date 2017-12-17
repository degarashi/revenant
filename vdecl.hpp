#pragma once
#include "vertex.hpp"
#include "spine/optional.hpp"
#include "gl_buffer.hpp"
#include "glx_const.hpp"

namespace rev {
	// 主にGLEffectからVDeclへのデータ受け渡しで使われる
	struct VData {
		using BuffA = const spi::Optional<draw::Buffer> (&)[MaxVStream];

		// [StreamIndex] -> Buffer(optional)
		BuffA				buff;
		// [VSemanticsIndex] -> AttributeId
		const VSemAttrV&	attr;

		VData(BuffA b, const VSemAttrV& at):
			buff(b),
			attr(at)
		{}
	};
	//! 頂点宣言
	class VDecl {
		private:
			template <class Ar>
			friend void save(Ar&, const VDecl&);
			template <class Ar>
			friend void load(Ar&, const VDecl&);
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

				bool operator == (const VDInfo& v) const;
				bool operator != (const VDInfo& v) const;
			};
		private:
			using VDInfoV = std::vector<VDInfo>;
			using Func = std::function<void (GLuint, const VSemAttrV&)>;
			using FuncV = std::vector<Func>;
			FuncV		_func;
			// ストリーム毎のサイズを1次元配列で格納 = 0番から並べる
			// 各ストリームの先頭インデックス
			int			_entIdx[MaxVStream+1];
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
			void apply(const VData& vdata) const;
			bool operator == (const VDecl& vd) const;
			bool operator != (const VDecl& vd) const;
	};
	using VDecl_SP = std::shared_ptr<VDecl>;
}
