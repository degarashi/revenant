#pragma once
#include "gltf/dc_common.hpp"
#include "gltf/idtag.hpp"
#include "../dc/common.hpp"
#include "lubee/fwd.hpp"
#include "../debuggui_if.hpp"

namespace rev::gltf {
	struct NodeParam_USem;
	struct RTUniform :
		IResolvable,
		IDebugGui
	{
		virtual ~RTUniform() {}
		virtual draw::Token_SP makeToken(
					dc::JointId currentId,
					const dc::SkinBindV_SP& bind,
					const frea::Mat4& bsm,
					const NodeParam_USem& np
				) const = 0;
		void resolve(const ITagQuery&) override {}
	};
}
