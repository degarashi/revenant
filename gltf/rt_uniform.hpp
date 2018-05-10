#pragma once
#include "gltf/dc_common.hpp"
#include "gltf/idtag.hpp"
#include "../dc/common.hpp"
#include "lubee/fwd.hpp"
#include "../debuggui_if.hpp"

namespace rev {
	class UniformEnt;
}
namespace rev::gltf {
	struct NodeParam_USem;
	struct RTUniform :
		IResolvable,
		IDebugGui
	{
		virtual ~RTUniform() {}
		virtual void exportUniform(
					UniformEnt& u,
					const SName& uname,
					dc::JointId currentId,
					const dc::SkinBindV_SP& bind,
					const frea::Mat4& bsm,
					const NodeParam_USem& np
				) const = 0;
		void resolve(const ITagQuery&) override {}
	};
}
