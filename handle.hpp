#pragma once
#include <memory>

namespace rev {
	#define DEF_HANDLE(typ, name) \
		using H##name = std::shared_ptr<typ>; \
		using W##name = std::weak_ptr<typ>;
	struct Resource;
	DEF_HANDLE(Resource, Res)

	class RWops;
	DEF_HANDLE(RWops, RW)
	class Camera2D;
	DEF_HANDLE(Camera2D, Cam2)
	class Camera3D;
	DEF_HANDLE(Camera3D, Cam3)

	struct Input;
	DEF_HANDLE(Input, Input)
	class Action;
	DEF_HANDLE(Action, Act)
	struct TPos2D;
	DEF_HANDLE(TPos2D, Ptr)
	class ABuffer;
	DEF_HANDLE(ABuffer, Ab)
	class ASource;
	DEF_HANDLE(ASource, Ss)
	class AGroup;
	DEF_HANDLE(AGroup, Sg)

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

	class FTFace;
	DEF_HANDLE(FTFace, FT)
	class TextObj;
	DEF_HANDLE(TextObj, Text)

	class IObject;
	DEF_HANDLE(IObject, Obj)
	class IUpdGroup;
	DEF_HANDLE(IUpdGroup, Group)
	class DrawableObj;
	DEF_HANDLE(DrawableObj, DObj)
	class DrawGroup;
	DEF_HANDLE(DrawGroup, DGroup)
	class IScene;
	DEF_HANDLE(IScene, Scene)
	class TechPass;
	DEF_HANDLE(TechPass, TP)

	namespace parse {
		struct GLXStruct;
	}
	DEF_HANDLE(parse::GLXStruct, Block)
	#undef DEF_HANDLE
}
