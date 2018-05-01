#pragma once
#include "frea/matrix.hpp"

namespace rev::dc {
	using Name = std::string;
	using Name_SP = std::shared_ptr<Name>;
	using Mat4 = frea::Mat4;
	using JointId = uint32_t;

	struct SkinBind {
		Name_SP		jointName;
		Mat4		invmat;

		bool operator == (const SkinBind& s) const noexcept;
	};
	using SkinBindV = std::vector<SkinBind>;
	using SkinBindV_SP = std::shared_ptr<SkinBindV>;

	using Mat4V = std::vector<Mat4>;
	struct NodeParam {
		virtual ~NodeParam() {}
		virtual Mat4 getLocal(JointId id) const = 0;
		virtual Mat4 getGlobal(JointId id) const = 0;
		virtual Mat4 getLocal(const Name& name) const = 0;
		virtual Mat4 getGlobal(const Name& name) const = 0;
		virtual const Mat4V& getJointMat(const Mat4& node_m, const SkinBindV_SP& bind, const Mat4& bs_m) const = 0;
	};
}

namespace std {
	template <>
	struct hash<rev::dc::SkinBind> {
		std::size_t operator()(const rev::dc::SkinBind& b) const noexcept {
			return std::hash<const rev::dc::Name*>()(b.jointName.get()) +
				std::hash<rev::dc::Mat4>()(b.invmat);
		}
	};
}
