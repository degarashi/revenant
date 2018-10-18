#pragma once
#include "frea/src/matrix.hpp"
#include "spine/src/flyweight_item.hpp"

namespace rev::dc {
	using SName = spi::FlyweightItem<std::string>;
	using Mat4 = frea::Mat4;
	using JointId = uint32_t;

	struct IQueryMatrix {
		virtual ~IQueryMatrix() {}
		virtual Mat4 getLocal(JointId id) const = 0;
		virtual Mat4 getGlobal(JointId id) const = 0;
		virtual Mat4 getLocal(const SName& name) const = 0;
		virtual Mat4 getGlobal(const SName& name) const = 0;
	};
}
