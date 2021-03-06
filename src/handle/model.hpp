#pragma once
#include "common.hpp"

namespace rev {
	namespace dc {
		class TfRoot;
		class TfNode;
		struct IModel;
		struct IMesh;
	}
	DEF_HANDLE(dc::TfRoot, Tf)
	DEF_HANDLE(dc::TfNode, TfNode)
	DEF_HANDLE(dc::IModel, Mdl)
	DEF_HANDLE(dc::IMesh, Mesh)
}
