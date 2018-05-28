#pragma once
#include "json_types.hpp"
#include "frea/angle.hpp"
#include <boost/variant.hpp>

namespace rev {
	class Camera3D;
	using HCam3 = std::shared_ptr<Camera3D>;
}
namespace rev::gltf {
	struct Camera {
		struct Perspective {
			float		aspectRatio;
			frea::RadF	yfov;
			float		zfar,
						znear;
			Perspective(const JValue& v);
		};
		struct Orthographic {
			float	xmag,
					ymag,
					zfar,
					znear;
			Orthographic(const JValue& v);
		};
		using Value = boost::variant<Perspective, Orthographic>;
		Value	value;

		Camera(const JValue& v);
		Value _init(const JValue& v);
		// デフォルト姿勢のカメラを作成
		// (Perspectiveのみの対応)
		HCam3 makeCamera() const;
	};
}
