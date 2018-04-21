#pragma once
#include "gltf/dc_common.hpp"
#include "gltf/idtag.hpp"
#include "../dc/common.hpp"
#include "../handle/camera.hpp"
#include "lubee/fwd.hpp"
#include "../debuggui_if.hpp"

namespace rev::gltf {
	struct RTUniform :
		IResolvable,
		IDebugGui
	{
		virtual ~RTUniform() {}
		virtual draw::Token_SP makeToken(
					const HCam3& c,
					const lubee::RectF& vp,
					dc::JointId currentId,
					const dc::SkinBindV_SP& bind,
					const frea::Mat4& bsm,
					const dc::NodeParam& param
				) const = 0;
		void resolve(const ITagQuery&) override {}
	};
}
