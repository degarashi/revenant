#pragma once
#include "frea/matrix.hpp"
#include "spine/flyweight_item.hpp"

namespace rev::dc {
	using SName = spi::FlyweightItem<std::string>;
	using Mat4 = frea::Mat4;
	using JointId = uint32_t;

	struct SkinBind {
		SName		jointName;
		Mat4		invmat;

		bool operator == (const SkinBind& s) const noexcept;
		std::size_t operator()(const SkinBind& s) const noexcept;
	};
	struct SkinBindSet {
		using SkinBindV = std::vector<SkinBind>;
		SkinBindV	bind;
		Mat4		bs_m;
	};
	using SkinBindSet_SP = std::shared_ptr<SkinBindSet>;

	using Mat4V = std::vector<Mat4>;
	struct NodeParam {
		virtual ~NodeParam() {}
		virtual Mat4 getLocal(JointId id) const = 0;
		virtual Mat4 getGlobal(JointId id) const = 0;
		virtual Mat4 getLocal(const SName& name) const = 0;
		virtual Mat4 getGlobal(const SName& name) const = 0;
		virtual const Mat4V& getJointMat(const Mat4& node_m, const SkinBindSet_SP& bind) const = 0;
	};
}

namespace std {
	template <>
	struct hash<rev::dc::SkinBind> {
		std::size_t operator()(const rev::dc::SkinBind& b) const noexcept {
			return std::hash<rev::dc::SName>()(b.jointName) +
				std::hash<rev::dc::Mat4>()(b.invmat);
		}
	};
}
