#pragma once
#include "common.hpp"

namespace rev {
	namespace dc {
		class TfRoot;
		class TfNode;
		struct IModel;
		struct IMesh;
		struct IChannel;
		struct IJointAt;
		struct IPoseSampler;
		struct IPosSampler;
	}
	DEF_HANDLE(dc::TfRoot, Tf)
	DEF_HANDLE(dc::TfNode, TfNode)
	DEF_HANDLE(dc::IModel, Mdl)
	DEF_HANDLE(dc::IMesh, Mesh)
	DEF_HANDLE(dc::IChannel, Channel)
	DEF_HANDLE(dc::IJointAt, Jat)
	DEF_HANDLE(dc::IPoseSampler, PoseSampler)
	DEF_HANDLE(dc::IPosSampler, PosSampler)
}
