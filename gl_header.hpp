#pragma once

// OpenGL関数プロトタイプは自分で定義するのでマクロを解除しておく
#undef GL_GLEXT_PROTOTYPES
#ifdef ANDROID
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
#else
	#include <GL/gl.h>
	#include "glext.h"
#endif

#if defined(WIN32)
	#include <GL/glcorearb.h>
#endif
