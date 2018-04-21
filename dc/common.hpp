#pragma once
#include "frea/matrix.hpp"

namespace rev::dc {
	using Name = std::string;
	using Name_SP = std::shared_ptr<Name>;
	using Mat4 = frea::Mat4;
	using JointId = uint32_t;

	struct NodeParam {
		virtual ~NodeParam() {}
		virtual Mat4 getLocal(JointId id) const = 0;
		virtual Mat4 getGlobal(JointId id) const = 0;
		virtual Mat4 getLocal(const Name& name) const = 0;
		virtual Mat4 getGlobal(const Name& name) const = 0;
	};
	struct SkinBind {
		Name_SP		jointName;
		Mat4		invmat;
	};
	using SkinBindV = std::vector<SkinBind>;
	using SkinBindV_SP = std::shared_ptr<SkinBindV>;
}
