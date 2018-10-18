#pragma once
#include "../../resmgr_app.hpp"
#include "../scenebase_obj.hpp"
#include "../../dc/animation.hpp"
#include "../../gltf/visitor_camera.hpp"
#include "../../handle/model.hpp"
#include "../fpcamera.hpp"

namespace rev::test {
	class glTF1Scene :
		public TestSceneObj<glTF1Scene>
	{
		private:
			FPCamera	_fpc;

			using CameraV = gltf::NodeVisitor_CameraBase::CameraV;
			StrV					_fileList,
									_fileFullPath;
			mutable CameraV			_camera;
			mutable std::size_t		_cameraIndex;
			mutable HMdl			_model;
			mutable dc::Animation	_anim;
			mutable bool			_dirtyFlag=false;

			struct St_Default;
			void _loadModel(const std::string& path) const;

		public:
			glTF1Scene();
	};
}
