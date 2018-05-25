#pragma once
#include "gltf/dc_common.hpp"
#include "gltf/dataref.hpp"
#include "lubee/fwd.hpp"
#include "../debuggui_if.hpp"

namespace rev::gltf {
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
