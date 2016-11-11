#pragma once
#include <memory>

namespace rev {
	struct Resource {
		virtual ~Resource() {}
		virtual const char* getResourceName() const noexcept = 0;
	};
	namespace resource_name {
		#define DEF_NAME(name) constexpr const char* name = #name
		DEF_NAME(RWops);
		DEF_NAME(TPos2D);
		DEF_NAME(Action);
		DEF_NAME(Input);
		DEF_NAME(ABuffer);
		DEF_NAME(ASource);
		DEF_NAME(AGroup);
		#undef DEF_NAME
	}
	struct ResourceNotFound : std::runtime_error {
		ResourceNotFound(const std::string& name):
			std::runtime_error("resource: " + name + "is not found")
		{}
	};

	#define DEF_HANDLE(typ, name) \
		using H##name = std::shared_ptr<typ>; \
		using W##name = std::weak_ptr<typ>;
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
	struct IEffect;
	DEF_HANDLE(IEffect, Fx)
	class GLFBuffer;
	DEF_HANDLE(GLFBuffer, Fb)
	class GLRBuffer;
	DEF_HANDLE(GLRBuffer, Rb)

	struct GLXStruct;
	DEF_HANDLE(GLXStruct, Block)

	#undef DEF_HANDLE

	using Priority = uint32_t;
	class URI;
	class UriHandlerV;
	class ResMgrBase {
		private:
			static UriHandlerV	s_handler;
		public:
			virtual HRes loadResource(const URI& uri) = 0;
	};
}
