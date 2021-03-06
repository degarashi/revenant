#include "gltf/camera.hpp"
#include "gltf/value_loader.hpp"
#include "gltf/check.hpp"
#include "../camera3d.hpp"
#include "../ovr_functor.hpp"

namespace rev::gltf {
	using namespace loader;
	// ---------------------- Camera::Perspective ----------------------
	Camera::Perspective::Perspective(const JValue& v):
		aspectRatio(OptionalDefault<Number>(v, "aspectRatio", 1.f)),
		yfov(Required<Number>(v, "yfov")),
		zfar(Required<Number>(v, "zfar")),
		znear(Required<Number>(v, "znear"))
	{
		CheckRange(aspectRatio, 0.f);
		CheckRange(yfov.get(), 0.f);
		CheckRange(zfar, znear);
		CheckRange(znear, 1e-7f);
	}
	// ---------------------- Camera::Orthographic ----------------------
	Camera::Orthographic::Orthographic(const JValue& v):
		xmag(Required<Number>(v, "xmag")),
		ymag(Required<Number>(v, "ymag")),
		zfar(Required<Number>(v, "zfar")),
		znear(Required<Number>(v, "znear"))
	{
		CheckRange(zfar, 0.f);
		CheckRange(znear, 0.f);
	}
	// ---------------------- Camera ----------------------
	namespace {
		const std::string c_type[] = {
			"perspective", "orthographic"
		};
	}
	Camera::Value Camera::_init(const JValue& v) {
		const std::string typ(Required<String>(v, "type"));
		const auto typpos = &CheckEnum(c_type, typ) - c_type;
		if(typpos == 0)
			return Perspective(v[c_type[0].c_str()]);
		return Orthographic(v[c_type[1].c_str()]);
	}
	Camera::Camera(const JValue& v):
		value(_init(v))
	{}
	HCam3 Camera::makeCamera() const {
		return boost::apply_visitor(OVR_Functor{
			[](const Perspective& p) {
				auto c = Camera3D::NewS();
				c->setFov(p.yfov);
				c->setAspect(p.aspectRatio);
				c->setZPlane(p.znear, p.zfar);
				return c;
			},
			[](const Orthographic&) {
				AssertF("orthographic camera is not supported");
				return Camera3D::NewS();
			}
		}, value);
	}
}
