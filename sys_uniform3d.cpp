#include "sys_uniform.hpp"
#include "sys_uniform_value.hpp"
#include "camera3d.hpp"
#include "glx_if.hpp"

namespace rev {
	SystemUniform3D::Getter::counter_t SystemUniform3D::Getter::operator()(const HCam3& c, Camera*, const SystemUniform3D&) const {
		if(c)
			return c->getAccum();
		return 0;
	}

	using GlxId = IEffect::GlxId;
	namespace sysunif3d {
		namespace matrix {
			const IdValue	Transform = GlxId::GenUnifId("sys_mTrans"),
							TransformInv = GlxId::GenUnifId("sys_mTransInv"),
							Proj = GlxId::GenUnifId("sys_mProj"),
							ProjInv = GlxId::GenUnifId("sys_mProjInv"),
							View = GlxId::GenUnifId("sys_mView"),
							ViewInv = GlxId::GenUnifId("sys_mViewInv"),
							ViewProj = GlxId::GenUnifId("sys_mViewProj"),
							ViewProjInv = GlxId::GenUnifId("sys_mViewProjInv"),
							World = GlxId::GenUnifId("sys_mWorld"),
							WorldInv = GlxId::GenUnifId("sys_mWorldInv"),
							EyePos = GlxId::GenUnifId("sys_vEyePos"),
							EyeDir = GlxId::GenUnifId("sys_vEyeDir");
		}
	}
	namespace unif3d {
		namespace texture {
			const IdValue	Specular = GlxId::GenUnifId("u_texSpecular"),
							Normal = GlxId::GenUnifId("u_texNormal"),
							Emissive = GlxId::GenUnifId("u_texEmissive");
		}
	}

	bool SystemUniform3D::_refresh(typename ViewInv::value_t& m, ViewInv*) const {
		auto ret = _rflag.getWithCheck(this, m);
		auto& cam = *std::get<0>(ret);
		const bool b = ret.flag;
		if(b) {
			m = cam->getView().convert<4,4>();
			m.invert();
		}
		return true;
	}
	bool SystemUniform3D::_refresh(typename ProjInv::value_t& m, ProjInv*) const {
		auto ret = _rflag.getWithCheck(this, m);
		auto& cam = *std::get<0>(ret);
		const bool b = ret.flag;
		if(b) {
			m = cam->getProj().convert<4,4>();
			m.invert();
		}
		return true;
	}
	bool SystemUniform3D::_refresh(typename Transform::value_t& m, Transform*) const {
		auto ret = _rflag.getWithCheck(this, m);
		auto& cam = *std::get<1>(ret);
		const bool b = ret.flag;
		if(b) {
			m = getWorld();
			if(cam)
				m *= cam->getViewProj();
		}
		return true;
	}
	bool SystemUniform3D::_refresh(frea::AMat4& m, TransformInv*) const {
		m = getTransform().inversion();
		return true;
	}
	bool SystemUniform3D::_refresh(frea::AMat4& m, WorldInv*) const {
		m = getWorld().inversion();
		return true;
	}

	SystemUniform3D::SystemUniform3D() {
		const auto im = frea::AMat4::Identity();
		setWorld(im);
		setTransform(im);
	}
	void SystemUniform3D::moveFrom(SystemUniform3D& prev) {
		_rflag = prev._rflag;
	}
	void SystemUniform3D::outputUniforms(IEffect& e) const {
		#define DEF_SETUNIF(name, func) \
			if(auto idv = e.getUnifId(sysunif3d::matrix::name)) \
				e.setUniform(*idv, spi::AcWrapperValue(func##name()), true);
		DEF_SETUNIF(World, get)
		DEF_SETUNIF(WorldInv, get)
		if(auto& hc = getCamera()) {
			auto& cd = *hc;
			DEF_SETUNIF(View, cd.get)
			DEF_SETUNIF(ViewInv, get)
			DEF_SETUNIF(Proj, cd.get)
			DEF_SETUNIF(ProjInv, get)
			DEF_SETUNIF(ViewProj, cd.get)
			DEF_SETUNIF(ViewProjInv, cd.get)

			auto& ps = cd.getPose();
			if(auto idv = e.getUnifId(sysunif3d::matrix::EyePos))
				e.setUniform(*idv, ps.getOffset(), true);
			if(auto idv = e.getUnifId(sysunif3d::matrix::EyeDir))
				e.setUniform(*idv, ps.getRotation().getZAxis(), true);
		}
		DEF_SETUNIF(Transform, get)
		DEF_SETUNIF(TransformInv, get)
		#undef DEF_SETUNIF
	}
}

