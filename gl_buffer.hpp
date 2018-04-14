#pragma once
#include "gl_types.hpp"
#include "drawtoken/token.hpp"
#include "debuggui_if.hpp"
#include <vector>

namespace rev {
	namespace draw {
		class Buffer;
		class Stream;
	}
	class GLBufferCore {
		private:
			template <class T>
			friend class RUser;
			friend class draw::Buffer;
			friend class draw::Stream;
			void use_begin() const;
			void use_end() const;
		protected:
			BufferType	_buffType;			//!< VERTEX_BUFFERなど
			DrawType	_drawType;			//!< STATIC_DRAWなどのフラグ
			GLuint		_stride,			//!< 要素1つのバイトサイズ
						_idBuff;			//!< OpenGLバッファID
		public:
			GLBufferCore(BufferType typ, DrawType dtype);
			virtual ~GLBufferCore() {}

			GLuint getBuffId() const;
			BufferType getBuffType() const;
			GLuint getStride() const;
	};

	// バッファのDrawTokenはVDeclとの兼ね合いからそのままリストに積まずに
	// StreamTagで一旦処理するのでスマートポインタではなく直接出力する
	//! OpenGLバッファクラス
	class GLBuffer :
		public IGLResource,
		public GLBufferCore,
		public std::enable_shared_from_this<GLBuffer>
	{
		private:
			template <class T>
			struct is_vector : std::false_type {};
			template <class T>
			struct is_vector<std::vector<T>> : std::true_type {};
		private:
			using SPBuff = std::shared_ptr<void>;
			SPBuff			_buff;			//!< 再構築の際に必要となるデータ実体(std::vector<T>)
			void*			_pBuffer;		//!< bufferの先頭ポインタ
			GLuint			_buffSize;		//!< bufferのバイトサイズ
			void	_initData();

			template <class T>
			using Decay = typename std::decay<T>::type;
			template <class T>
			using ChkIfVector = typename std::enable_if<is_vector<Decay<T>>::value>::type;
			template <class T, class = ChkIfVector<T>>
			void _setVec(T&& src, const GLuint stride) {
				using Vec = Decay<T>;
				const auto deleter = [](void* ptr) {
					auto* rt = reinterpret_cast<Vec*>(ptr);
					delete rt;
				};
				_stride = stride;
				_buffSize = src.size() * sizeof(typename Vec::value_type);
				Vec* rt = new Vec(std::forward<T>(src));
				_pBuffer = rt->data();
				_buff = SPBuff(static_cast<void*>(rt), deleter);
			}
		protected:
			void* _getBufferPtr() const noexcept;
			std::size_t _getBufferSize() const noexcept;
		public:
			GLBuffer(BufferType typ, DrawType dtype);
			GLBuffer(const GLBuffer&) = delete;
			~GLBuffer() override;

			// 全域を書き換え
			void initData(const void* src, std::size_t nElem, GLuint stride);
			template <class T, class = ChkIfVector<T>>
			void initData(T&& src, GLuint stride=sizeof(typename Decay<T>::value_type)) {
				_setVec(std::forward<T>(src), stride);
				_initData();
			}
			// 部分的に書き換え
			void updateData(const void* src, std::size_t nElem, GLuint offset);
			GLuint getSize() const;
			GLuint getNElem() const;

			void onDeviceLost() override;
			void onDeviceReset() override;
			draw::Buffer getDrawToken() const;

			DEF_DEBUGGUI_PROP
	};

	//! 頂点バッファ
	class GLVBuffer : public GLBuffer {
		public:
			GLVBuffer(DrawType dtype);
			DEF_DEBUGGUI_NAME
	};
	//! インデックスバッファ
	class GLIBuffer : public GLBuffer {
		public:
			GLIBuffer(DrawType dtype);
			using GLBuffer::initData;
			void initData(const GLubyte* src, std::size_t nElem);
			void initData(const GLushort* src, std::size_t nElem);

			void updateData(const GLushort* src, std::size_t nElem, GLuint offset);
			void updateData(const GLubyte* src, std::size_t nElem, GLuint offset);
			GLenum getSizeFlag() const;
			static GLenum GetSizeFlag(int stride);
			#ifdef DEBUGGUI_ENABLED
				template <class T>
				bool _property(int columns, const char* typestr, bool edit);
			#endif
			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
