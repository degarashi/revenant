#pragma once
#include "token.hpp"
#include "buffer.hpp"
#include "../vdecl.hpp"
#include "../glx_const.hpp"

namespace rev {
	namespace draw {
		class UserFunc : public TokenT<UserFunc> {
			public:
				using Func = std::function<void ()>;
			private:
				Func	_func;
			public:
				UserFunc(const Func& f);
				void exec() override;
		};
		//MEMO: ソースの改変を経ている為、少し実装が回りくどいと思われる
		//! 頂点インプットに関するOpenGLコール
		/*! 頂点バッファ, インデックスバッファ, 頂点フォーマットが対象 */
		class Stream {
			private:
				friend class RUser<Stream>;
				void use_begin() const;
				void use_end() const;
			public:
				using Buffer_OP = spi::Optional<Buffer>;
				// vertex stream
				HVDecl		spVDecl;
				Buffer_OP	vbuff[MaxVStream];
				VSem_AttrV	vAttrId;
				// index stream
				Buffer_OP	ibuff;

				RUser<Stream> use();
		};
		template <class T>
		class DrawBase : public TokenT<T> {
			private:
				Stream		_stream;
			protected:
				DrawBase(Stream&& vs):
					_stream(std::move(vs))
				{}
				RUser<Stream> use() {
					return _stream.use();
				}
		};
		//! Draw token (without index)
		class Draw : public DrawBase<Draw> {
			private:
				GLenum		_mode;
				GLint		_first;
				GLsizei		_count;
			public:
				Draw(Stream&& vs, GLenum mode, GLint first, GLsizei count);
				void exec() override;
		};
		//! Draw token (with index)
		class DrawIndexed : public DrawBase<DrawIndexed> {
			private:
				GLenum		_mode;
				GLsizei		_count;
				GLenum		_sizeF;
				GLuint		_offset;
			public:
				/*! \param[in] mode 描画モードフラグ(OpenGL)
					\param[in] count 描画に使用される要素数
					\param[in] sizeF 1要素のサイズを表すフラグ
					\param[in] offset オフセットバイト数 */
				DrawIndexed(Stream&& vs, GLenum mode, GLsizei count, GLenum sizeF, GLuint offset);
				void exec() override;
		};
	}
}
