#pragma once
#include "../handle/model.hpp"
#include "../debug_gui/if.hpp"
#include "bbox.hpp"
#include <vector>

namespace rev {
	class IEffect;
}
namespace rev::dc {
	struct IModel :
		IDebugGui
	{
		virtual ~IModel() {}
		virtual void draw(IEffect& e) const = 0;
		virtual HTf getNode() const = 0;
		virtual dc::BSphere_Op getBSphere() const { return spi::none; }
	};
}
