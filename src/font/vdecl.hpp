#pragma once
#include "../vdecl.hpp"
#include "frea/src/vector.hpp"

namespace rev::vertex {
	//! 文字列描画用頂点
	struct text {
		frea::Vec2	pos;
		frea::Vec3	uvt;

		const static FWVDecl s_decl;
	};
}
