#pragma once
#include "../handle/model.hpp"
#include "../debuggui_if.hpp"
#include <vector>

namespace rev {
	class IEffect;
}
namespace rev::dc {
	class IModel :
		public IDebugGui
	{
		protected:
			using MeshV = std::vector<HMesh>;
			MeshV		_mesh;
			HTf			_tf;

		public:
			virtual ~IModel() {}
			virtual void draw(IEffect& e) const = 0;
			const HTf& getNode() const noexcept;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
