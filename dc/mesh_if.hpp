#pragma once
#include "../handle/opengl.hpp"
#include "../debuggui_if.hpp"

namespace rev {
	class IEffect;
}
namespace rev::dc {
	struct NodeParam;
	struct IMesh :
		IDebugGui
	{
		virtual ~IMesh() {}
		virtual void draw(IEffect& e, const NodeParam& np) const = 0;
		virtual HTech getTech() const = 0;
	};
}
