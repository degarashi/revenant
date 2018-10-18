#pragma once
#include "../../resmgr_app.hpp"
#include "../../gltf/visitor_camera.hpp"
#include "../../handle/model.hpp"
#include "../../dc/animation.hpp"
#include "../scenebase_obj.hpp"
#include "../fpcamera.hpp"
#include "litrot.hpp"
#include "skybox.hpp"

namespace rev::test {
	class glTF2Scene :
		public TestSceneObj<glTF2Scene>
	{
		private:
			using CameraV = gltf::NodeVisitor_CameraBase::CameraV;
			using Idx_OP = spi::Optional<std::size_t>;
			StrV			_fileList,
							_fileFullPath;
			CameraV			_camera;
			HMdl			_model;
			dc::Animation	_anim;
			mutable Idx_OP	_toLoad,
							_nextCamera;
			std::size_t		_cameraIndex;

			FPCamera		_fpc;
			LitRotObj		_lit;
			HCam3			_activeC;
			HAct			_actCameraSw,
							_actCameraZoom;
			Skybox			_skybox;

			struct St_TFBase : StateT<St_TFBase, St_Base> {
				void onUpdate(glTF2Scene& self) override;
				void onDraw(const glTF2Scene& self, IEffect& e) const override;
			};
			// ----- 一人称カメラ -----
			struct St_FPCamera : StateT<St_FPCamera, St_TFBase> {
				void onEnter(glTF2Scene& self, ObjTypeId_OP) override;
				void onUpdate(glTF2Scene& self) override;
			};
			// ----- glTFモデルカメラ -----
			struct St_MCamera : StateT<St_MCamera, St_TFBase> {
				void onEnter(glTF2Scene& self, ObjTypeId_OP) override;
				void onUpdate(glTF2Scene& self) override;
			};
			// ----- オブジェクト注視カメラ -----
			struct St_ObjCamera : StateT<St_ObjCamera, St_TFBase> {
				HCam3					_camera;

				struct Obj {
					frea::Quat				rot;
					frea::Vec3				center;
					float					radius,
											dist;
				};
				using Obj_Op = spi::Optional<Obj>;
				Obj_Op		_obj;

				bool			_rotating;

				static float _CalcDist(float radius, frea::RadF fov);
				void _calcCamera();
				St_ObjCamera();
				void onEnter(glTF2Scene& self, ObjTypeId_OP) override;
				void onExit(glTF2Scene& self, ObjTypeId_OP) override;
				void onUpdate(glTF2Scene& self) override;
			};

			void _loadModel(const std::string& path);
			bool _checkLoadModel();
			// -> _toLoad
			void _modelSelector() const;
			// -> _nextCamera
			void _cameraSelector() const;

		public:
			glTF2Scene();
	};
}
