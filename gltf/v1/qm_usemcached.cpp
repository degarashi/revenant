#include "qm_usemcached.hpp"
#include "../../camera3d.hpp"
#include "../../uniform_ent.hpp"
#include "semantic_if.hpp"
#include "lubee/hash_combine.hpp"

namespace rev::gltf::v1 {
	// --------------------- QueryMatrix_USemCached::USemKey ---------------------
	std::size_t QueryMatrix_USemCached::USemKey::getHash() const noexcept {
		return std::hash<dc::JointId>()(jointId) + std::hash<USemantic::e>()(sem);
	}
	bool QueryMatrix_USemCached::USemKey::operator == (const USemKey& k) const noexcept {
		return jointId == k.jointId &&
				sem == k.sem;
	}
	// --------------------- QueryMatrix_USemCached::SkinKey ---------------------
	std::size_t QueryMatrix_USemCached::SkinKey::getHash() const noexcept {
		return lubee::hash_combine_implicit(jointId, bind);
	}
	bool QueryMatrix_USemCached::SkinKey::operator == (const SkinKey& k) const noexcept {
		return jointId == k.jointId &&
				bind == k.bind;
	}

	// --------------------- QueryMatrix_USemCached ---------------------
	QueryMatrix_USemCached::QueryMatrix_USemCached(const HCam3& cam, const lubee::RectF& vp, const dc::IQueryMatrix& qm):
		_qm(qm),
		_matrix(
			[this](const USemKey& k) -> Mat4 {
				const auto local = [this, id=k.jointId]() -> Mat4 { return _qm.getLocal(id); };
				const auto global = [this, id=k.jointId]() -> Mat4 { return _qm.getGlobal(id); };
				switch(k.sem) {
					case USemantic::View:
						return _view;
					case USemantic::Projection:
						return _proj;
					case USemantic::Local:
						// This is the node's matrix property
						return local();
					case USemantic::Model:
						// Transforms from model to world coordinates using the transform's node and all of its ancestors
						return global();
					case USemantic::ModelView:
						// Combined MODEL and VIEW
						return global() * _view;
					case USemantic::ModelViewProjection:
						// Combined MODEL, VIEW, and PROJECTION
						return global() * _viewProj ;
					case USemantic::ModelInverse:
						// Inverse of MODEL
						return global().inversion();
					case USemantic::ViewInverse:
						// Inverse of VIEW
						return _view.inversion();
					case USemantic::ProjectionInverse:
						// Inverse of PROJECTION
						return _proj.inversion();
					case USemantic::ModelViewInverse:
						// Inverse of MODELVIEW
						return _matrix.getCache(USemKey{k.jointId, USemantic::ModelView}).inversion();
					case USemantic::ModelViewProjectionInverse:
						// Inverse of MODELVIEWPROJECTION
						return _matrix.getCache(USemKey{k.jointId, USemantic::ModelViewProjection}).inversion();
					case USemantic::ModelInverseTranspose:
						// This translates normals in model coordinates to world coordinates
						return _matrix.getCache(USemKey{k.jointId, USemantic::Model});
					case USemantic::ModelViewInverseTranspose:
						// This translates normals in model coordinates to eye coordinates
						return _matrix.getCache(USemKey{k.jointId, USemantic::ModelView});
					default:
						Assert0(false);
				}
				return {};
			}
		),
		_skin([this](const SkinKey& k){
			const auto len = k.bind->bind.size();
			Mat4V ret(len);
			const Mat4 node_m = getGlobal(k.jointId);
			for(std::size_t i=0 ; i<len ; i++) {
				auto& b = k.bind->bind[i];
				ret[i] = k.bind->bs_m * b.invmat * getGlobal(b.jointName) * node_m;
			}
			return ret;
		})
	{
		_camera = cam;
		_viewport = Vec4{vp.x0, vp.y0, vp.width(), vp.height()};
		_view = _camera->getView() * Mat4::Scaling({1,1,-1,1});
		_proj = Mat4::Scaling({1,1,-1,1}) * _camera->getProj();
		_viewProj = _view * _proj;
	}
	void QueryMatrix_USemCached::exportSemantic(ISemanticSet& s, const JointId id, const SkinBindSet_SP& bind, const USemantic sem) const {
		if(sem == USemantic::Viewport) {
			s.set(_viewport);
			return;
		}
		if(sem == USemantic::JointMatrix) {
			s.set(_skin.getCache(SkinKey{.jointId=id, .bind=bind}), false);
			return;
		}
		if(sem == USemantic::View) {
			// Transforms from world to view coordinates using the active camera node
			s.set(_view, false);
			return;
		}
		if(sem == USemantic::Projection) {
			// Transforms from view to clip coordinates using the active camera node
			s.set(_proj, false);
			return;
		}

		const auto& m = _matrix.getCache(USemKey{.jointId=id, .sem=sem});
		if(sem == USemantic::ModelInverseTranspose ||
			sem == USemantic::ModelViewInverseTranspose)
			s.set(frea::Mat3(m.convert<3,3>()), false);
		else
			s.set(m, false);
	}
	dc::Mat4 QueryMatrix_USemCached::getLocal(const JointId id) const {
		return _qm.getLocal(id);
	}
	dc::Mat4 QueryMatrix_USemCached::getGlobal(const JointId id) const  {
		return _qm.getGlobal(id);
	}
	dc::Mat4 QueryMatrix_USemCached::getLocal(const dc::SName& name) const  {
		return _qm.getLocal(name);
	}
	dc::Mat4 QueryMatrix_USemCached::getGlobal(const dc::SName& name) const  {
		return _qm.getGlobal(name);
	}
}
