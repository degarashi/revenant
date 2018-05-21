#include "node_cached.hpp"
#include "../camera3d.hpp"
#include "../uniform_ent.hpp"

namespace rev::gltf {
	std::size_t NodeParam_USemCached::USemKey::operator()(const USemKey& k) const noexcept {
		return std::hash<dc::JointId>()(k.jointId) + std::hash<USemantic::e>()(k.sem);
	}
	bool NodeParam_USemCached::USemKey::operator == (const USemKey& k) const noexcept {
		return jointId == k.jointId &&
				sem == k.sem;
	}
	NodeParam_USemCached::NodeParam_USemCached(const HCam3& cam, const lubee::RectF& vp, dc::NodeParam& np):
		_np(np)
	{
		_camera = cam;
		_viewport = Vec4{vp.x0, vp.y0, vp.width(), vp.height()};
		_view = _camera->getView() * Mat4::Scaling({1,1,-1,1});
		_proj = Mat4::Scaling({1,1,-1,1}) * _camera->getProj();
		_viewProj = _view * _proj;
	}
	NodeParam_USemCached::Mat4 NodeParam_USemCached::_calcMat(const JointId id, const USemantic sem) const {
		if(sem == USemantic::View) {
			return _view;
		}
		if(sem == USemantic::Projection) {
			return _proj;
		}
		const auto makeMat = [this, id, sem]() -> Mat4 {
			const auto local = [this, id]() -> Mat4 { return _np.getLocal(id); };
			const auto global = [this, id]() -> Mat4 { return _np.getGlobal(id); };
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
	void NodeParam_USemCached::exportSemantic(UniformEnt& u, const SName& uname, const JointId id, const USemantic sem) const {
		if(sem == USemantic::View) {
			// Transforms from world to view coordinates using the active camera node
			u.setUniform(uname, _view, false);
			return;
		}
		if(sem == USemantic::Projection) {
			// Transforms from view to clip coordinates using the active camera node
			u.setUniform(uname, _proj, false);
			return;
		}

		const auto m = _calcMat(id, sem);
		if(sem == USemantic::ModelInverseTranspose ||
			sem == USemantic::ModelViewInverseTranspose)
			u.setUniform(uname, frea::Mat3(m.convert<3,3>()), false);
		else
			u.setUniform(uname, frea::Mat4(m), false);
	}
	void NodeParam_USemCached::exportViewport(UniformEnt& u, const SName& uname) const {
		u.setUniform(uname, _viewport);
	}
	dc::Mat4 NodeParam_USemCached::getLocal(const JointId id) const {
		return _np.getLocal(id);
	}
	dc::Mat4 NodeParam_USemCached::getGlobal(const JointId id) const  {
		return _np.getGlobal(id);
	}
	dc::Mat4 NodeParam_USemCached::getLocal(const dc::SName& name) const  {
		return _np.getLocal(name);
	}
	dc::Mat4 NodeParam_USemCached::getGlobal(const dc::SName& name) const  {
		return _np.getGlobal(name);
	}
	const dc::Mat4V& NodeParam_USemCached::getJointMat(const dc::Mat4& node_m, const dc::SkinBindSet_SP& bind) const {
		return _np.getJointMat(node_m, bind);
	}
}
