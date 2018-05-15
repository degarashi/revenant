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
	NodeParam_USemCached::NodeParam_USemCached(const HCam3& cam, const lubee::RectF& vp, const NodeParam& np):
		dc::NodeParam_cached(np)
	{
		_camera = cam;
		_viewport = Vec4{vp.x0, vp.y0, vp.width(), vp.height()};
		_view = Mat4::Scaling({1,1,-1,1}) * _camera->getView().transposition();
		_proj = _camera->getProj().transposition() * Mat4::Scaling({1,1,-1,1});
		_viewProj = _proj * _view;
	}
	NodeParam_USemCached::Mat4 NodeParam_USemCached::_calcMat(const JointId id, const USemantic sem) const {
		if(sem == USemantic::View) {
			return _view;
		}
		if(sem == USemantic::Projection) {
			return _proj;
		}
		const auto makeMat = [this, id, sem]() -> Mat4 {
			const auto local = [this, id]() -> Mat4 { return base_t::getLocal(id).transposition(); };
			const auto global = [this, id]() -> Mat4 { return base_t::getGlobal(id).transposition(); };
			switch(sem) {
				case USemantic::Local:
					// This is the node's matrix property
					return local();
				case USemantic::Model:
					// Transforms from model to world coordinates using the transform's node and all of its ancestors
					return global();
				case USemantic::ModelView:
					// Combined MODEL and VIEW
					return _view * global();
				case USemantic::ModelViewProjection:
					// Combined MODEL, VIEW, and PROJECTION
					return _viewProj * global();
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
			u.setUniform(uname, _view);
			return;
		}
		if(sem == USemantic::Projection) {
			// Transforms from view to clip coordinates using the active camera node
			u.setUniform(uname, _proj);
			return;
		}

		const auto m = _calcMat(id, sem);
		if(sem == USemantic::ModelInverseTranspose ||
			sem == USemantic::ModelViewInverseTranspose)
			u.setUniform(uname, frea::Mat3(m.convert<3,3>()));
		else
			u.setUniform(uname, frea::Mat4(m));
	}
	void NodeParam_USemCached::exportViewport(UniformEnt& u, const SName& uname) const {
		u.setUniform(uname, _viewport);
	}
	dc::Mat4 NodeParam_USemCached::getLocal(const JointId id) const {
		return base_t::getLocal(id);
	}
	dc::Mat4 NodeParam_USemCached::getGlobal(const JointId id) const  {
		return base_t::getGlobal(id);
	}
	dc::Mat4 NodeParam_USemCached::getLocal(const dc::SName& name) const  {
		return base_t::getLocal(name);
	}
	dc::Mat4 NodeParam_USemCached::getGlobal(const dc::SName& name) const  {
		return base_t::getGlobal(name);
	}
	const dc::Mat4V& NodeParam_USemCached::getJointMat(const dc::Mat4& node_m, const dc::SkinBindV_SP& bind, const dc::Mat4& bs_m) const {
		return base_t::getJointMat(node_m, bind, bs_m);
	}
}
