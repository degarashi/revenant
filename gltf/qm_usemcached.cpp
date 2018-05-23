#include "qm_usemcached.hpp"
#include "../camera3d.hpp"
#include "../uniform_ent.hpp"
#include "semantic_if.hpp"

namespace rev::gltf {
	std::size_t QueryMatrix_USemCached::USemKey::operator()(const USemKey& k) const noexcept {
		return std::hash<dc::JointId>()(k.jointId) + std::hash<USemantic::e>()(k.sem);
	}
	bool QueryMatrix_USemCached::USemKey::operator == (const USemKey& k) const noexcept {
		return jointId == k.jointId &&
				sem == k.sem;
	}
	QueryMatrix_USemCached::QueryMatrix_USemCached(const HCam3& cam, const lubee::RectF& vp, const dc::IQueryMatrix& qm):
		_qm(qm)
	{
		_camera = cam;
		_viewport = Vec4{vp.x0, vp.y0, vp.width(), vp.height()};
		_view = _camera->getView() * Mat4::Scaling({1,1,-1,1});
		_proj = Mat4::Scaling({1,1,-1,1}) * _camera->getProj();
		_viewProj = _view * _proj;
	}
	QueryMatrix_USemCached::Mat4 QueryMatrix_USemCached::_calcMat(const JointId id, const USemantic sem) const {
		if(sem == USemantic::View) {
			return _view;
		}
		if(sem == USemantic::Projection) {
			return _proj;
		}
		const auto makeMat = [this, id, sem]() -> Mat4 {
			const auto local = [this, id]() -> Mat4 { return _qm.getLocal(id); };
			const auto global = [this, id]() -> Mat4 { return _qm.getGlobal(id); };
			switch(sem) {
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
					return _calcMat(id, USemantic::ModelView).inversion();
				case USemantic::ModelViewProjectionInverse:
					// Inverse of MODELVIEWPROJECTION
					return _calcMat(id, USemantic::ModelViewProjection).inversion();
				case USemantic::ModelInverseTranspose:
					// This translates normals in model coordinates to world coordinates
					return _calcMat(id, USemantic::Model);
				case USemantic::ModelViewInverseTranspose:
					// This translates normals in model coordinates to eye coordinates
					return _calcMat(id, USemantic::ModelView);
				default:
					Assert0(false);
			}
			return {};
		};

		const USemKey key{
			.jointId = id,
			.sem = sem
		};
		auto itr = _matrix.find(key);
		if(itr == _matrix.end()) {
			itr = _matrix.emplace(key, makeMat()).first;
		}
		return itr->second;
	}
	void QueryMatrix_USemCached::exportSemantic(ISemanticSet& s, const JointId id, const USemantic sem) const {
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

		const auto m = _calcMat(id, sem);
		if(sem == USemantic::ModelInverseTranspose ||
			sem == USemantic::ModelViewInverseTranspose)
			s.set(frea::Mat3(m.convert<3,3>()), false);
		else
			s.set(frea::Mat4(m), false);
	}
	void QueryMatrix_USemCached::exportViewport(ISemanticSet& s) const {
		s.set(_viewport);
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
	const Mat4V& QueryMatrix_USemCached::getJointMat(const Mat4& node_m, const SkinBindSet_SP& bind) const {
		const auto len = bind->bind.size();
		_jointMat.resize(len);
		auto node_t = node_m.transposition();
		for(std::size_t i=0 ; i<len ; i++) {
			auto& b = bind->bind[i];
			_jointMat[i] = node_t * getGlobal(b.jointName).transposition() * b.invmat * bind->bs_m;
		}
		return _jointMat;
	}
}
