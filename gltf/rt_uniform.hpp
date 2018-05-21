#pragma once
#include "gltf/dc_common.hpp"
#include "gltf/idtag.hpp"
#include "../dc/common.hpp"
#include "lubee/fwd.hpp"
#include "../debuggui_if.hpp"

namespace rev::gltf {
	struct NodeParam_USem;
	struct ISemanticSet;
	struct RTUniform :
		IResolvable,
		IDebugGui
	{
		virtual ~RTUniform() {}
		virtual void exportUniform(
					ISemanticSet& s,
					dc::JointId currentId,
					const dc::SkinBindSet_SP& bind,
					const NodeParam_USem& np
				) const = 0;
		void resolve(const ITagQuery&) override {}
	};
}
