#include "sys_uniform2d.hpp"
#include "sys_uniform_value.hpp"
#include "glx_if.hpp"
#include "camera2d.hpp"
#include "gl_program.hpp"
#include "drawtoken/make_uniform.hpp"
#include "uniform_ent.hpp"

namespace rev {
	SystemUniform2D::Getter::counter_t SystemUniform2D::Getter::operator()(const HCam2& c, Camera*, const SystemUniform2D&) const {
		if(c)
			return c->getAccum();
		return 0;
	}

	namespace unif2d {
		const LiteralStr
							Depth("u_depth");
	}
	namespace sysunif2d {
		namespace matrix {
			const LiteralStr
							World("sys_mWorld2d"),
							WorldInv("sys_mWorld2dInv"),
							View("sys_mView2d"),
							ViewInv("sys_mView2dInv"),
							Proj("sys_mProj2d"),
							ProjInv("sys_mProj2dInv"),
							ViewProj("sys_mViewProj2d"),
							ViewProjInv("sys_mViewProj2dInv"),
							Transform("sys_mTrans2d"),
							TransformInv("sys_mTrans2dInv");
		}
	}
	bool SystemUniform2D::_refresh(frea::Mat3& m, WorldInv*) const {
		m = getWorld().inversion();
		return true;
	}
	bool SystemUniform2D::_refresh(typename Transform::value_t& m, Transform*) const {
		auto ret = _rflag.getWithCheck(this, m);
		auto& cam = *std::get<1>(ret);
		const bool b = ret.flag;
		if(b) {
			m = getWorld();
			if(cam)
				m *= cam->getViewProj();
		}
		return b;
	}
	bool SystemUniform2D::_refresh(frea::Mat3& m, TransformInv*) const {
		m = getTransform().inversion();
		return true;
	}

	SystemUniform2D::SystemUniform2D() {
		auto im = frea::Mat3::Identity();
		setWorld(im);
		setTransform(im);
	}
	void SystemUniform2D::moveFrom(ISystemUniform& prev) {
		auto& p = dynamic_cast<SystemUniform2D&>(prev);
		_rflag = p._rflag;
	}
	void SystemUniform2D::outputUniforms(UniformEnt& u) const {
		#define DEF_SETUNIF(name, func) \
			u.setUniformWithMake(sysunif2d::matrix::name, [&](){ \
				return draw::MakeUniform(spi::UnwrapAcValue(func##name())); \
			});
		DEF_SETUNIF(World, get)
		DEF_SETUNIF(WorldInv, get)
		if(auto& hc = getCamera()) {
			auto& cd = *hc;
			DEF_SETUNIF(View, cd.get)
			DEF_SETUNIF(ViewInv, cd.get)
			DEF_SETUNIF(Proj, cd.get)
			DEF_SETUNIF(ViewProj, cd.get)
			DEF_SETUNIF(ViewProjInv, cd.get)
		}
		DEF_SETUNIF(Transform, get)
		DEF_SETUNIF(TransformInv, get)
		#undef DEF_SETUNIF
	}
}
