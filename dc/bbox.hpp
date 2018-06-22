#pragma once
#include "frea/matrix.hpp"
#include "spine/optional.hpp"

namespace rev::dc {
	struct BSphere {
		frea::Vec3	center;
		float		radius;
	};
	using BSphere_Op = spi::Optional<BSphere>;
	struct BBox {
		frea::Vec3	min, max;

		void append(const BBox& b);
		BSphere makeSphere() const;
		BBox transform(const frea::AMat4& m) const;
		bool valid() const noexcept;
	};
	using BBox_Op = spi::Optional<BBox>;
}
