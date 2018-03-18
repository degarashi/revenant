#pragma once
#include "common.hpp"

namespace rev {
	struct IGLResource;
	DEF_HANDLE(IGLResource, GLRes)
	class IGLTexture;
	DEF_HANDLE(IGLTexture, Tex)
	class Texture_Mem;
	DEF_HANDLE(Texture_Mem, TexMem)
	class Texture_URI;
	DEF_HANDLE(Texture_URI, TexURI)
	class Texture_CubeURI;
	DEF_HANDLE(Texture_CubeURI, TexCubeURI)
	class GLVBuffer;
	DEF_HANDLE(GLVBuffer, Vb)
	class GLIBuffer;
	DEF_HANDLE(GLIBuffer, Ib)
	class GLBuffer;
	DEF_HANDLE(GLBuffer, Buff)
	class GLProgram;
	DEF_HANDLE(GLProgram, Prog)
	class GLShader;
	DEF_HANDLE(GLShader, Sh)
	class IEffect;
	DEF_HANDLE(IEffect, Fx)
	class GLFBuffer;
	DEF_HANDLE(GLFBuffer, Fb)
	class GLRBuffer;
	DEF_HANDLE(GLRBuffer, Rb)

	struct GLState;
	DEF_HANDLE(GLState, GLState)
	class VDecl;
	DEF_HANDLE(VDecl, VDecl)
	struct ITech;
	DEF_HANDLE(ITech, Tech)
	struct Primitive;
	DEF_HANDLE(Primitive, Prim)
	class TechPass;
	DEF_HANDLE(TechPass, TP)
	namespace parse {
		struct GLXStruct;
	}
	DEF_HANDLE(parse::GLXStruct, Block)
}
