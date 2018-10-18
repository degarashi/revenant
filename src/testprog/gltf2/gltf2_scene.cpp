#include "gltf2_scene.hpp"
#include "gltf/v2/gltf.hpp"
#include "gltf/v2/dc_model.hpp"
#include "../../imgui_sdl2.hpp"
#include "../../debug_gui/resource_window.hpp"
#include "../../debug_gui/window.hpp"
#include "../../debug_gui/id.hpp"
#include "../../imgui/imgui.h"
#include "../../dc/node.hpp"
#include "../../u_matrix3d.hpp"
#include "../../glx_if.hpp"
#include "../../camera3d.hpp"
#include "../../u_pbr_mr.hpp"
#include "shared.hpp"

namespace rev::test {
	using T2 = glTF2Scene;
	// -------------- glTF2Scene::St_TFBase --------------
	void T2::St_TFBase::onUpdate(T2& self) {
		St_Base::onUpdate(self);

		const auto& pose = self._activeC->getPose();
		self._lit.update(pose.getRight(), pose.getUp());

		if(!self._anim.empty()) {
			auto& node = *self._model->getNode();
			constexpr float dt = 1.f / 60;
			const auto ended = self._anim.advance(dt);
			self._anim.update(node);
			if(ended)
				self._anim.loop();
		}
	}
	void T2::St_TFBase::onDraw(const T2& self, IEffect& e) const {
		St_Base::onDraw(self, e);
		self._skybox.draw(e);
		{
			auto& pbr = dynamic_cast<U_PBRMetallicRoughness&>(e);
			pbr.lightDir = self._lit.getDir();
		}
		{
			auto& u3 = dynamic_cast<U_Matrix3D&>(e);
			u3.setCamera(self._activeC);
		}
		self._modelSelector();
		self._cameraSelector();
		if(self._model)
			self._model->draw(e);
	}

	// -------------- glTF2Scene::St_FPCamera --------------
	void T2::St_FPCamera::onEnter(T2& self, const ObjTypeId_OP id) {
		St_TFBase::onEnter(self, id);
		self._activeC = self._fpc.getCamera();
		self._cameraIndex = 0;
	}
	void T2::St_FPCamera::onUpdate(T2& self) {
		{
			auto& sh = *tls_shared;
			self._fpc.update(
				sh.act[UserShare::Act::CMoveX],
				sh.act[UserShare::Act::CMoveY],
				sh.act[UserShare::Act::CMoveZ],
				sh.act[UserShare::Act::CDirX],
				sh.act[UserShare::Act::CDirY],
				sh.act[UserShare::Act::CDirBtn]
			);
		}
		if(const auto id = self._nextCamera) {
			self._cameraIndex = *id;
			self._nextCamera = spi::none;
			if(self._cameraIndex != 0)
				self.setStateNew<St_MCamera>();
		}
		self._checkLoadModel();
		St_TFBase::onUpdate(self);

		if(self._actCameraSw->isKeyPressed()) {
			self.setStateNew<St_ObjCamera>();
		}
	}

	// -------------- glTF2Scene::St_MCamera --------------
	void T2::St_MCamera::onEnter(T2& self, ObjTypeId_OP) {
		auto& idToNode = self._model->getNode()->getIdToNode();
		auto& info = self._camera[self._cameraIndex-1];
		frea::Mat4 m4 = idToNode.at(info.id)->getTransform();
		m4 *= frea::Mat4::Scaling({1,1,-1});
		beat::g3::Pose pose(m4);
		auto& r = pose.refRotation();
		r.z *= -1;
		r.w *= -1;
		pose.refOffset().z *= -1;
		info.camera->setPose(pose);
		info.camera->setAspect(self._fpc.getCamera()->getAspect());
		self._activeC = info.camera;
	}
	void T2::St_MCamera::onUpdate(T2& self) {
		if(const auto id = self._nextCamera) {
			self._nextCamera = spi::none;
			self._cameraIndex = *id;
			if(self._cameraIndex != 0)
				self.setStateNew<St_MCamera>();
			else
				self.setStateNew<St_FPCamera>();
		}
		St_TFBase::onUpdate(self);
	}
	namespace {
		constexpr frea::DegF c_fov(60.f);
	}
	// -------------- glTF2Scene::St_ObjCamera --------------
	T2::St_ObjCamera::St_ObjCamera():
		_camera(std::make_shared<Camera3D>())
	{
		_camera->setFov(c_fov);
		_camera->setZPlane(1e-2f, 1e3f);
	}
	void T2::St_ObjCamera::onEnter(T2& self, ObjTypeId_OP) {
		_camera->setAspect(self._fpc.getCamera()->getAspect());
		self._activeC = _camera;
	}
	void T2::St_ObjCamera::onExit(T2& self, ObjTypeId_OP) {
		if(self._model) {
			auto tf = self._model->getNode();
			tf->refNode()[0]->refPose().setRotation(frea::Quat::Identity());
		}
	}
	float T2::St_ObjCamera::_CalcDist(const float radius, const frea::RadF fov) {
		const float w = std::tan(fov.get()/2);
		frea::Vec2 dir(w, 1);
		dir.normalize();
		dir = frea::Vec2(dir.y, -dir.x);
		const frea::Vec2 cp(dir * radius);
		float dist = std::sqrt(cp.len_sq() + radius*radius) * 0.5f;
		dist = std::max(radius*0.25f, dist);
		return dist;
	}
	void T2::St_ObjCamera::_calcCamera() {
		if(_obj) {
			auto& obj = *_obj;
			auto& pose = _camera->refPose();
			const frea::Vec3 ofs = obj.center - frea::Vec3(0,0, obj.dist);
			pose.setOffset(ofs);
			const auto rot = frea::Quat::LookAt(obj.center - ofs, {0,1,0});
			pose.setRotation(rot);
		}
	}
	void T2::St_ObjCamera::onUpdate(T2& self) {
		St_TFBase::onUpdate(self);
		if(self._checkLoadModel())
			_obj = spi::none;
		if(self._model) {
			if(!_obj) {
				if(const auto bs = self._model->getBSphere()) {
					_obj = Obj{
						.rot = frea::Quat::Identity(),
						.center = bs->center,
						.radius = bs->radius,
						.dist = _CalcDist(bs->radius, c_fov)
					};
				}
			} else {
				if(const auto w = self._actCameraZoom->getValue();
					w != 0 &&
					!mgr_gui.pointerOnGUI()
				)
				{
					_obj->dist += -w*_obj->radius*1e-1f;
				}
				_calcCamera();
			}
		}

		auto& sh = *tls_shared;
		if(_rotating) {
			if(!sh.act[UserShare::Act::CDirBtn]->isKeyPressing()) {
				const auto hM = Mouse::OpenMouse(0);
				hM->setMouseMode(MouseMode::Absolute);
				_rotating = false;
			}
			if(_obj && self._model) {
				const float dx = sh.act[UserShare::Act::CDirX]->getValueAsFloat() * 70,
							dy = sh.act[UserShare::Act::CDirY]->getValueAsFloat() * 70;
				const auto& pose = self._activeC->getPose();
				const auto right = pose.getRight();
				const auto up = pose.getUp();
				const auto q = frea::Quat::Rotation(up, frea::DegF(-dx)) *
								frea::Quat::Rotation(right, frea::DegF(-dy));
				auto& obj = *_obj;
				obj.rot = q * obj.rot;
				auto tf = self._model->getNode();
				tf->refNode()[0]->refPose().setRotation(obj.rot);
			}
		} else {
			if(sh.act[UserShare::Act::CDirBtn]->isKeyPressed() &&
				!mgr_gui.pointerOnGUI())
			{
				const auto hM = Mouse::OpenMouse(0);
				hM->setMouseMode(MouseMode::Relative);
				_rotating = true;
			}
			if(self._actCameraSw->isKeyPressed()) {
				self.setStateNew<St_FPCamera>();
				return;
			}
		}
	}

	// -------------- glTF2Scene --------------
	bool T2::_checkLoadModel() {
		if(_toLoad) {
			const auto idx = *_toLoad;
			_toLoad = spi::none;
			_loadModel(_fileFullPath[idx]);
			return true;
		}
		return false;
	}
	void T2::_loadModel(const std::string& path) {
		auto g = gltf::v2::GLTF::Make(FileURI(path));
		{
			auto& sc = g.defaultScene ? *(*g.defaultScene) : *(g.m_Scene[0]);
			// モデル
			_model = gltf::v2::GLTFModel::FromScene(sc);
			// カメラリスト
			gltf::NodeVisitor_Camera<gltf::v2::Node> c;
			for(auto& n : sc.node) {
				n->visit(c);
			}
			_camera = c.result();
			std::sort(_camera.begin(), _camera.end(), [](const auto& e0, const auto& e1){
				return e0.name < e1.name;
			});
			// アニメーション
			_anim.clear();
			for(auto& a : g.m_Animation)
				_anim.append(a->makeAnimation());

			// BBox
		}
		// モデル詳細GUIを表示
		debug::ResourceWindow::Add(_model);
	}
	void T2::_modelSelector() const {
		if(const auto _ = debug::WindowPush("model select")) {
			auto* st = ImGui::GetStateStorage();
			int* sel = st->GetIntRef(ImGui::GetID(""), -1);
			int idx=0;
			for(auto& f : _fileList) {
				const auto _ = debug::IdPush(idx);
				if(ImGui::Selectable(f.c_str(), idx==*sel)) {
					*sel = idx;
					_toLoad = idx;
				}
				++idx;
			}
		}
	}
	void T2::_cameraSelector() const {
		if(_model) {
			if(const auto _ = debug::WindowPush("camera select")) {
				std::size_t idx=0;
				if(ImGui::Selectable("User", _cameraIndex==idx)) {
					_nextCamera = idx;
				}
				++idx;
				for(auto& c : _camera) {
					const auto _ = debug::IdPush(idx);
					if(ImGui::Selectable(c.name.c_str(), _cameraIndex==idx)) {
						_nextCamera = idx;
					}
					++idx;
				}
			}
		}
	}
	glTF2Scene::glTF2Scene() {
		setStateNew<St_ObjCamera>();
		_makeGui();

		// サンプルglTFファイルを列挙
		_fileFullPath = Dir::EnumEntryWildCard(Dir::GetProgramDir() + "/resource/gltf_sample/2.0/*/glTF/*.gltf");
		std::sort(_fileFullPath.begin(), _fileFullPath.end());
		const std::size_t len = _fileFullPath.size();
		_fileList.resize(len);
		for(std::size_t i=0 ; i<len ; i++) {
			const PathBlock pb(_fileFullPath[i]);
			_fileList[i] = pb.getLast_utf8();
		}
		_actCameraSw = mgr_input.makeAction("camera-switch");
		const auto hK = Keyboard::OpenKeyboard();
		_actCameraSw->addLink(hK, InputFlag::Button, VKey::Q);

		const auto hM = Mouse::OpenMouse(0);
		_actCameraZoom = mgr_input.makeAction("camera-zoom");
		_actCameraZoom->addLink(hM, InputFlag::Axis, 3);
	}
}
