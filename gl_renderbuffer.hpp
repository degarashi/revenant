#pragma once
#include "gl_types.hpp"
#include "frea/vector.hpp"
#include "gl_format.hpp"
#include "lubee/size.hpp"
#include <boost/variant.hpp>
#include <vector>

namespace rev {
	using ByteBuff = std::vector<uint8_t>;
	class GLFBufferTmp;
	//! OpenGL: RenderBufferObjectインタフェース
	/*! Use/EndインタフェースはユーザーがRenderbufferに直接何かする事は無いが内部的に使用 */
	class GLRBuffer : public IGLResource {
		public:
			//! DeviceLost時の挙動
			enum OnLost {
				NONE,		//!< 何もしない(領域はゴミデータになる)
				CLEAR,		//!< 単色でクリア
				RESTORE,	//!< 事前に保存しておいた内容で復元
				NUM_ONLOST
			};
			friend class RUser<GLRBuffer>;
			void use_begin() const;
			void use_end() const;
			// OpenGL ES2.0だとglDrawPixelsが使えない
		private:
			using F_LOST = std::function<void (GLFBufferTmp&,GLRBuffer&)>;
			const static F_LOST cs_onLost[NUM_ONLOST],
								cs_onReset[NUM_ONLOST];
			GLuint		_idRbo;
			OnLost		_behLost;

			using Res = boost::variant<boost::blank, frea::Vec4, ByteBuff>;
			Res				_restoreInfo;
			GLTypeFmt		_buffFmt;
			GLFormatV		_fmt;
			lubee::SizeI	_size;
			//! 現在指定されているサイズとフォーマットでRenderbuffer領域を確保 (内部用)
			void allocate();

		public:
			GLRBuffer(int w, int h, GLInRenderFmt fmt);
			~GLRBuffer();
			void onDeviceReset() override;
			void onDeviceLost() override;

			void setOnLost(OnLost beh, const frea::Vec4* color=nullptr);
			GLuint getBufferId() const;
			const lubee::SizeI& getSize() const;
			const GLFormatV& getFormat() const;
			const char* getResourceName() const noexcept override;
	};
}
#include "luaimport.hpp"
DEF_LUAIMPORT(rev::GLRBuffer)
