#pragma once
#include "frea/matrix.hpp"

namespace rev::gltf {
	struct ISemanticSet {
		virtual ~ISemanticSet() {}
		virtual void set(const frea::Vec4& v) = 0;
		virtual void set(const frea::Mat3& m, bool t) = 0;
		virtual void set(const frea::Mat4& m, bool t) = 0;
		virtual void set(const std::vector<frea::Mat4>& mv, bool t) = 0;
	};
}
