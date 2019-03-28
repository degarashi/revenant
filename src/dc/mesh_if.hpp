#pragma once
#include "../handle/opengl.hpp"
#include "../debug_gui/if.hpp"

namespace rev {
	class IEffect;
}
namespace rev::dc {
	struct IQueryMatrix;
	struct IMesh :
		IDebugGui
	{
		virtual ~IMesh() {}
		virtual HTech getTech() const = 0;
	};
}
