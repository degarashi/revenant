#pragma once
#include "../handle/model.hpp"
#include "../debuggui_if.hpp"
#include <vector>

namespace rev {
	class IEffect;
}
namespace rev::dc {
	class Model :
		public IDebugGui
	{
		private:
			using MeshV = std::vector<HMesh>;
			MeshV		_mesh;
			HTf			_tf;
		public:
			Model(const MeshV& mesh, const HTf& tf);
			void draw(IEffect& e) const;
			const HTf& getNode() const noexcept;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
