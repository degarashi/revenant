#pragma once
#include "common.hpp"

namespace rev {
	namespace dc {
		class TfRoot;
		class TfNode;
		class IModel;
		class IMesh;
		struct IChannel;
		struct IJointAt;
		struct ISampler;
		struct IPosSampler;
	}
	DEF_HANDLE(dc::TfRoot, Tf)
	DEF_HANDLE(dc::TfNode, TfNode)
	DEF_HANDLE(dc::IModel, Mdl)
	DEF_HANDLE(dc::IMesh, Mesh)
	DEF_HANDLE(dc::IChannel, Channel)
	DEF_HANDLE(dc::IJointAt, Jat)
	DEF_HANDLE(dc::ISampler, Sampler)
	DEF_HANDLE(dc::IPosSampler, PosSampler)
}
