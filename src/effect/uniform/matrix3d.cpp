#include "matrix3d.hpp"
#include "../../camera3d.hpp"
#include "../if.hpp"
#include "../uniform_ent.hpp"
#include "../../gl/program.hpp"
#include "spine/src/flyweight_item.hpp"

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
							EyeDir("sys_vEyeDir"),
							Joint("sys_mJoint");
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
	bool U_Matrix3D::_refresh(M4& m, TransformInv*) const {
		m = getTransform().inversion();
		return true;
	}
	bool U_Matrix3D::_refresh(M4& m, WorldInv*) const {
		m = getWorld().inversion();
		return true;
	}

	U_Matrix3D::U_Matrix3D() {
		const auto im = M4::Identity();
		setWorld(im);
		setTransform(im);
	}
	void U_Matrix3D::resetWorld() {
		setWorld(M4::Identity());
	}
	namespace {
		using M4 = frea::AMat4;
		const M4& GetIdentityMat4() {
			const static M4 mat = M4::Identity();
			return mat;
		}
	}
	UniformSetF U_Matrix3D::getUniformF(const GLProgram& prog) const {
		UniformSetF_V fv;

		const auto set = [&fv, &prog](const auto& name, const auto& cb) {
			if(const auto id = prog.getUniformId(name)) {
				fv.emplace_back([id=*id, &cb](const void* p, UniformEnt& u){
					auto* self = static_cast<const U_Matrix3D*>(p);
					u.setUniformById(id, spi::UnwrapAcValue(cb(*self)));
				});
			}
		};
		#define DEF_SETUNIF(name) \
			set(s3d::name, [](auto& s)->decltype(auto){ return s.get##name(); });
		DEF_SETUNIF(World)
		DEF_SETUNIF(WorldInv)
		DEF_SETUNIF(Transform)
		DEF_SETUNIF(TransformInv)
		DEF_SETUNIF(ViewInv)
		DEF_SETUNIF(ProjInv)
		DEF_SETUNIF(Joint)
		#undef DEF_SETUNIF

		#define DEF_SETUNIF(name) \
			set(s3d::name, [](auto& s)-> const M4& { \
				if(const auto& c = s.getCamera()) \
					return c->get##name(); \
				return GetIdentityMat4(); });
		DEF_SETUNIF(View)
		DEF_SETUNIF(Proj)
		DEF_SETUNIF(ViewProj)
		DEF_SETUNIF(ViewProjInv)
		#undef DEF_SETUNIF

		set(s3d::EyePos, [](auto& s)-> frea::Vec3 {
							if(const auto& c = s.getCamera())
								return c->getPose().getOffset();
							return {0,0,0}; });
		set(s3d::EyeDir, [](auto& s)-> frea::Vec3 {
							if(const auto& c = s.getCamera())
								return c->getPose().getRotation().getZAxis();
							return {0,1,0}; });

		if(!fv.empty()) {
			return [fv = std::move(fv)](const void* p, UniformEnt& u){
				for(auto& f : fv)
					f(p, u);
			};
		}
		return nullptr;
	}
}

