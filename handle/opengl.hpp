#pragma once
#include "common.hpp"
#include "spine/flyweight_item.hpp"

namespace rev {
	struct IGLResource;
	DEF_HANDLE(IGLResource, GLRes)

	class ITextureSource;
	DEF_HANDLE(ITextureSource, TexSrc)
	class GLTexture;
	DEF_HANDLE(GLTexture, Tex)
	class TextureSrc_Mem;
	DEF_HANDLE(TextureSrc_Mem, TexMem)
	class TextureSrc_Mem2D;
	DEF_HANDLE(TextureSrc_Mem2D, TexMem2D)
	class TextureSrc_MemCube;
	DEF_HANDLE(TextureSrc_MemCube, TexMemCube)
	class TextureSrc_URI;
	DEF_HANDLE(TextureSrc_URI, TexURI)
	class TextureSrc_CubeURI;
	DEF_HANDLE(TextureSrc_CubeURI, TexCubeURI)
	class TextureFilter;
	DEF_HANDLE(TextureFilter, TexF)

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
	struct VSemAttrMap;
	using FWVMap = spi::FlyweightItem<VSemAttrMap>;
	class VDecl;
	using FWVDecl = spi::FlyweightItem<VDecl>;
	struct ITech;
	DEF_HANDLE(ITech, Tech)
	class Primitive;
	DEF_HANDLE(Primitive, Prim)
	class TechPass;
	DEF_HANDLE(TechPass, TP)
	namespace parse {
		struct GLXStruct;
	}
	DEF_HANDLE(parse::GLXStruct, Block)
}
