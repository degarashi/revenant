#pragma once
#include "gl_header.hpp"
#include "spine/optional.hpp"
#include "spine/enum.hpp"
#include "debuggui_if.hpp"
#include "resource.hpp"

namespace rev {
	using GLboolean_OP = spi::Optional<GLboolean>;
	using GLint_OP = spi::Optional<GLint>;
	using GLuint_OP = spi::Optional<GLuint>;
	using GLfloat_OP = spi::Optional<GLfloat>;
	#ifndef USE_OPENGLES2
		using GLdouble_OP = spi::Optional<GLdouble>;
	#endif

	DefineEnum(
		ShType,
		(Vertex)
		(Geometry)
		(Fragment)
	);
	//! シェーダーIDに対するOpenGL定数
	const static GLuint c_glShFlag[ShType::_Num] = {
		GL_VERTEX_SHADER,
		#ifdef ANDROID
			0xdeadbeef,
		#else
			GL_GEOMETRY_SHADER,
		#endif
		GL_FRAGMENT_SHADER
	};
	DefineEnumPair(
		DrawMode,
		((Points)(GL_POINTS))
		((Lines)(GL_LINES))
		((LineStrip)(GL_LINE_STRIP))
		((LineLoop)(GL_LINE_LOOP))
		((Triangles)(GL_TRIANGLES))
		((TriangleStrip)(GL_TRIANGLE_STRIP))
		((TriangleFan)(GL_TRIANGLE_FAN))
		((Quads)(GL_QUADS))
		((QuadStrip)(GL_QUAD_STRIP))
		((Polygon)(GL_POLYGON))
	);
	DefineEnum(
		MipState,
		(NoMipmap)
		(MipmapNear)
		(MipmapLinear)
	);
	DefineEnum(
		WrapState,
		(ClampToEdge)
		(ClampToBorder)
		(MirroredRepeat)
		(Repeat)
		(MirrorClampToEdge)
	);
	DefineEnumPair(
		BufferType,
		((Vertex)(GL_ARRAY_BUFFER))
		((Index)(GL_ELEMENT_ARRAY_BUFFER))
	);
	DefineEnumPair(
		DrawType,
		((Static)(GL_STATIC_DRAW))
		((Dynamic)(GL_DYNAMIC_DRAW))
	);
	DefineEnum(
		CubeFace,
		(PositiveX)
		(NegativeX)
		(PositiveY)
		(NegativeY)
		(PositiveZ)
		(NegativeZ)
	);
 	//! OpenGL関連のリソース
	/*! Android用にデバイスロスト対応 */
	struct IGLResource :
		Resource,
		IDebugGui
	{
		virtual void onDeviceLost() {}
		virtual void onDeviceReset() {}
		virtual const char* getResourceName() const noexcept override;
	};
	template <class T>
	class RUser {
		private:
			const T&	_t;
			bool		_bRelease;
		public:
			RUser(RUser&& r) noexcept:
				_t(r._t),
				_bRelease(true)
			{
				r._bRelease = false;
			}
			RUser(const RUser&) = delete;
			RUser(const T& t):
				_t(t),
				_bRelease(true)
			{
				_t.use_begin();
			}
			~RUser() {
				if(_bRelease)
					_t.use_end();
			}
	};
	struct FBInfo {
		GLint		redSize,
					greenSize,
					blueSize,
					alphaSize,
					depthSize,
					stencilSize,
					id;
		bool		bTex;
	};
	using FBInfo_OP = spi::Optional<FBInfo>;
}
