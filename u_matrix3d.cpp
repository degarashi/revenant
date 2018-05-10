#include "u_matrix3d.hpp"
#include "camera3d.hpp"
#include "glx_if.hpp"
#include "gl_program.hpp"
#include "uniform_ent.hpp"
#include "spine/flyweight_item.hpp"

namespace rev {
	U_Matrix3D::Getter::counter_t U_Matrix3D::Getter::operator()(const HCam3& c, Camera*, const U_Matrix3D&) const {
		if(c)
			return c->getAccum();
		return 0;
	}

	using UniformName = spi::FlyweightItem<std::string>;
	namespace {
		namespace s3d {
			const UniformName
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
			const UniformName
							Specular("u_texSpecular"),
							Normal("u_texNormal"),
							Emissive("u_texEmissive");
		}
	}

	bool U_Matrix3D::_refresh(typename ViewInv::value_t& m, ViewInv*) const {
		auto ret = _rflag.getWithCheck(this, m);
		auto& cam = *std::get<0>(ret);
		const bool b = ret.flag;
		if(b) {
			m = cam->getView().convert<4,4>();
			m.invert();
		}
		return true;
	}
	bool U_Matrix3D::_refresh(typename ProjInv::value_t& m, ProjInv*) const {
		auto ret = _rflag.getWithCheck(this, m);
		auto& cam = *std::get<0>(ret);
		const bool b = ret.flag;
		if(b) {
			m = cam->getProj().convert<4,4>();
			m.invert();
		}
		return true;
	}
	bool U_Matrix3D::_refresh(typename Transform::value_t& m, Transform*) const {
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
	bool U_Matrix3D::_refresh(frea::AMat4& m, TransformInv*) const {
		m = getTransform().inversion();
		return true;
	}
	bool U_Matrix3D::_refresh(frea::AMat4& m, WorldInv*) const {
		m = getWorld().inversion();
		return true;
	}

	U_Matrix3D::U_Matrix3D() {
		const auto im = frea::AMat4::Identity();
		setWorld(im);
		setTransform(im);
	}
	UniformSetF U_Matrix3D::getUniformF(const GLProgram& prog) const {
		UniformSetF_V fv;

		#define DEF_SETUNIF(name) \
			if(const auto id = prog.getUniformId(s3d::name)) { \
				fv.emplace_back([id=*id](const void* p, UniformEnt& u){ \
					auto* self = static_cast<const U_Matrix3D*>(p); \
					u.setUniformById(id, spi::UnwrapAcValue(self->get##name())); \
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
			if(const auto id = prog.getUniformId(s3d::name)) { \
				fv.emplace_back([id=*id](const void* p, UniformEnt& u){ \
					auto* self = static_cast<const U_Matrix3D*>(p); \
					u.setUniformById(id, spi::UnwrapAcValue(self->getCamera()->get##name())); \
				}); \
			}
		DEF_SETUNIF(View)
		DEF_SETUNIF(Proj)
		DEF_SETUNIF(ViewProj)
		DEF_SETUNIF(ViewProjInv)
		#undef DEF_SETUNIF

		if(const auto id = prog.getUniformId(s3d::EyePos)) {
			fv.emplace_back([id=*id](const void* p, UniformEnt& u){
				auto* self = static_cast<const U_Matrix3D*>(p);
				auto& ps = self->getCamera()->getPose();
				u.setUniformById(id, ps.getOffset());
			});
		}
		if(const auto id = prog.getUniformId(s3d::EyePos)) {
			fv.emplace_back([id=*id](const void* p, UniformEnt& u){
				auto* self = static_cast<const U_Matrix3D*>(p);
				auto& ps = self->getCamera()->getPose();
				u.setUniformById(id, ps.getRotation().getZAxis());
			});
		}

		if(!fv.empty()) {
			return [fv = std::move(fv)](const void* p, UniformEnt& u){
				for(auto& f : fv)
					f(p, u);
			};
		}
		return nullptr;
	}
}

