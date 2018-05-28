#pragma once
#include "gltf/v1/dc_common.hpp"
#include "gltf/v1/dataref.hpp"
#include "lubee/fwd.hpp"
#include "../../debuggui_if.hpp"

namespace rev::gltf::v1 {
	struct IQueryMatrix_USem;
	struct ISemanticSet;
	struct RTUniform :
		IDebugGui
	{
		virtual ~RTUniform() {}
		virtual void exportUniform(
					ISemanticSet& s,
					dc::JointId currentId,
					const SkinBindSet_SP& bind,
					const IQueryMatrix_USem& qm
				) const = 0;
	};
}
