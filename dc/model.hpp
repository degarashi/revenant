#pragma once
#include "../handle/model.hpp"
#include "../debuggui_if.hpp"
#include <vector>

namespace rev {
	class IEffect;
}
namespace rev::dc {
	struct NodeParam;
	using NodeParam_UP = std::unique_ptr<NodeParam>;

	class Model :
		public IDebugGui
	{
		public:
			using MakeCacheF = NodeParam_UP (*)(const IEffect&, const NodeParam&);
		private:
			using MeshV = std::vector<HMesh>;
			MeshV		_mesh;
			HTf			_tf;
			MakeCacheF	_mc;

			static NodeParam_UP DefaultCache(const IEffect& e, const NodeParam& np);
		public:
			Model(const MeshV& mesh, const HTf& tf, MakeCacheF mc=&DefaultCache);
			void draw(IEffect& e) const;
			const HTf& getNode() const noexcept;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
