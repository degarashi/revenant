#include "u_matrix2d.hpp"
#include "glx_if.hpp"
#include "camera2d.hpp"
#include "gl/program.hpp"
#include "uniform_ent.hpp"
#include "spine/src/flyweight_item.hpp"

namespace rev {
	U_Matrix2D::Getter::counter_t U_Matrix2D::Getter::operator()(const HCam2& c, Camera*, const U_Matrix2D&) const {
		if(c)
			return c->getAccum();
		return 0;
	}

	using UniformName = spi::FlyweightItem<std::string>;
	namespace unif2d {
		const UniformName
							Depth("u_depth");
	}
	namespace {
		namespace s2d {
			const UniformName
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
	bool U_Matrix2D::_refresh(frea::Mat3& m, WorldInv*) const {
		m = getWorld().inversion();
		return true;
	}
	bool U_Matrix2D::_refresh(typename Transform::value_t& m, Transform*) const {
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
	bool U_Matrix2D::_refresh(frea::Mat3& m, TransformInv*) const {
		m = getTransform().inversion();
		return true;
	}

	U_Matrix2D::U_Matrix2D() {
		auto im = frea::Mat3::Identity();
		setWorld(im);
		setTransform(im);
	}
	UniformSetF U_Matrix2D::getUniformF(const GLProgram& prog) const {
		UniformSetF_V fv;

		#define DEF_SETUNIF(name) \
			if(const auto id = prog.getUniformId(s2d::name)) { \
				fv.emplace_back([id=*id](const void* p, UniformEnt& u){ \
					auto* self = static_cast<const U_Matrix2D*>(p); \
					u.setUniformById(id, spi::UnwrapAcValue(self->get##name())); \
				}); \
			}
		DEF_SETUNIF(World)
		DEF_SETUNIF(WorldInv)
		DEF_SETUNIF(Transform)
		DEF_SETUNIF(TransformInv)
		#undef DEF_SETUNIF

		#define DEF_SETUNIF(name) \
			if(const auto id = prog.getUniformId(s2d::name)) { \
				fv.emplace_back([id=*id](const void* p, UniformEnt& u){ \
					auto* self = static_cast<const U_Matrix2D*>(p); \
					u.setUniformById(id, spi::UnwrapAcValue(self->getCamera()->get##name())); \
				}); \
			}
		DEF_SETUNIF(View)
		DEF_SETUNIF(ViewInv)
		DEF_SETUNIF(Proj)
		DEF_SETUNIF(ViewProj)
		DEF_SETUNIF(ViewProjInv)
		#undef DEF_SETUNIF

		if(!fv.empty()) {
			return [fv = std::move(fv)](const void* p, UniformEnt& u) {
				for(auto& f : fv)
					f(p, u);
			};
		}
		return nullptr;
	}
}
