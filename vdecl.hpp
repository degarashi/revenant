#pragma once
#include "vertex.hpp"
#include "spine/optional.hpp"
#include "gl_buffer.hpp"

namespace rev {
	struct VData {
		const static int MAX_STREAM = 4;
		using BuffA = const spi::Optional<draw::Buffer> (&)[MAX_STREAM];
		using AttrA = GLint[static_cast<int>(VSem::_Num)];

		BuffA	buff;
		const AttrA&	attrId;

		VData(BuffA b, const AttrA& at): buff(b), attrId(at) {}
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
				GLuint	streamId,		//!< 便宜上の)ストリームId
						offset,			//!< バイトオフセット
						elemFlag,		//!< OpenGLの要素フラグ
						bNormalize,		//!< OpenGLが正規化するか(bool)
						elemSize,		//!< 要素数
						semId;			//!< 頂点セマンティクスId

				bool operator == (const VDInfo& v) const;
				bool operator != (const VDInfo& v) const;
			};
			using VDInfoV = std::vector<VDInfo>;
		private:
			using Func = std::function<void (GLuint, const VData::AttrA&)>;
			using FuncL = std::vector<Func>;
			FuncL		_func;						//!< ストリーム毎のサイズを1次元配列で格納 = 0番から並べる
			int			_nEnt[VData::MAX_STREAM+1];	//!< 各ストリームの先頭インデックス
			VDInfoV		_vdInfo;

			static VDInfoV _ToVector(std::initializer_list<VDInfo>& il);
			void _init();

		public:
			VDecl();
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
