#include "bbox.hpp"

namespace rev::dc {
	void BBox::append(const BBox& b) {
		min.selectMin(b.min);
		max.selectMax(b.max);
	}
	BSphere BBox::makeSphere() const {
		return {
			.center = (min + max)/2,
			.radius = (max-min).length()
		};
	}
	BBox BBox::transform(const frea::AMat4& m) const {
		const frea::Vec3 mm[2] = {min, max};
		frea::Vec4 tmp[8];
		auto* tmp_p = tmp;
		for(std::size_t iz=0 ; iz<2 ; iz++) {
			for(std::size_t iy=0 ; iy<2 ; iy++) {
				for(std::size_t ix=0 ; ix<2 ; ix++) {
					*tmp_p++ = frea::Vec4(mm[ix].x, mm[iy].y, mm[iz].z, 1);
				}
			}
		}
		D_Assert0(tmp_p == tmp+8);

		using L = std::numeric_limits<float>;
		frea::Vec4 minv(L::max()),
					maxv(L::min());
		for(auto& t : tmp) {
			t *= m;
			minv.selectMin(t);
			maxv.selectMax(t);
		}
		return {
			.min = minv.convert<3>(),
			.max = maxv.convert<3>()
		};
	}
	bool BBox::valid() const noexcept {
		for(std::size_t i=0 ; i<3 ; i++) {
			if(min[i] > max[i])
				return false;
		}
		return true;
	}
}
