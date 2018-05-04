#include "sys_uniform3d.hpp"
#include "sys_uniform_value.hpp"
#include "camera3d.hpp"
#include "glx_if.hpp"
#include "gl_program.hpp"
#include "drawtoken/make_uniform.hpp"
#include "uniform_ent.hpp"

namespace rev {
	SystemUniform3D::Getter::counter_t SystemUniform3D::Getter::operator()(const HCam3& c, Camera*, const SystemUniform3D&) const {
		if(c)
			return c->getAccum();
		return 0;
	}

	namespace sysunif3d {
		namespace matrix {
			const LiteralStr
							Transform("sys_mTrans"),
							TransformInv("sys_mTransInv"),
							Proj("sys_mProj"),
							ProjInv("sys_mProjInv"),
							View("sys_mView"),
							ViewInv("sys_mViewInv"),
							ViewProj("sys_mViewProj"),
							ViewProjInv("sys_mViewProjInv"),
							World("sys_mWorld"),
							WorldInv("sys_mWorldInv"),
							EyePos("sys_vEyePos"),
							EyeDir("sys_vEyeDir");
		}
	}
	namespace unif3d {
		namespace texture {
			const LiteralStr
							Specular("u_texSpecular"),
							Normal("u_texNormal"),
							Emissive("u_texEmissive");
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
	void SystemUniform3D::moveFrom(ISystemUniform& prev) {
		auto& p = dynamic_cast<SystemUniform3D&>(prev);
		_rflag = p._rflag;
	}
	void SystemUniform3D::extractUniform(UniformSetF_V& dst, const GLProgram& prog) const {
		#define DEF_SETUNIF(name) \
			if(const auto id = prog.getUniformId(sysunif3d::matrix::name)) { \
				dst.emplace_back([id=*id](const void* p, UniformEnt& u){ \
					auto* self = static_cast<const SystemUniform3D*>(p); \
					u.setUniform(id, spi::UnwrapAcValue(self->get##name())); \
				}); \
			}
		DEF_SETUNIF(World)
		DEF_SETUNIF(WorldInv)
		DEF_SETUNIF(Transform)
		DEF_SETUNIF(TransformInv)
		DEF_SETUNIF(ViewInv)
		DEF_SETUNIF(ProjInv)
		#undef DEF_SETUNIF

		#define DEF_SETUNIF(name) \
			if(const auto id = prog.getUniformId(sysunif3d::matrix::name)) { \
				dst.emplace_back([id=*id](const void* p, UniformEnt& u){ \
					auto* self = static_cast<const SystemUniform3D*>(p); \
					u.setUniform(id, spi::UnwrapAcValue(self->getCamera()->get##name())); \
				}); \
			}
		DEF_SETUNIF(View)
		DEF_SETUNIF(Proj)
		DEF_SETUNIF(ViewProj)
		DEF_SETUNIF(ViewProjInv)
		#undef DEF_SETUNIF

		if(const auto id = prog.getUniformId(sysunif3d::matrix::EyePos)) {
			dst.emplace_back([id=*id](const void* p, UniformEnt& u){
				auto* self = static_cast<const SystemUniform3D*>(p);
				auto& ps = self->getCamera()->getPose();
				u.setUniform(id, ps.getOffset());
			});
		}
		if(const auto id = prog.getUniformId(sysunif3d::matrix::EyePos)) {
			dst.emplace_back([id=*id](const void* p, UniformEnt& u){
				auto* self = static_cast<const SystemUniform3D*>(p);
				auto& ps = self->getCamera()->getPose();
				u.setUniform(id, ps.getRotation().getZAxis());
			});
		}
	}
}

