#pragma once
#include "myscene.hpp"
#include "../../gltf/visitor_camera.hpp"

namespace rev::test {
	struct MyScene::St_glTF : StateT<St_glTF, St_3D> {
		using CameraV = gltf::NodeVisitor_CameraBase::CameraV;
		StrV					_fileList,
								_fileFullPath;
		mutable HCam3			_cameraBkup;
		mutable CameraV			_camera;
		mutable std::size_t		_cameraIndex;
		mutable HMdl			_model;
		mutable dc::Animation	_anim;
		mutable bool			_dirtyFlag=false;

		void _loadModel(const std::string& path) const;
		void onEnter(MyScene& self, ObjTypeId_OP) override;
		void onDraw(const MyScene& self, IEffect& e) const override;
	};
}
