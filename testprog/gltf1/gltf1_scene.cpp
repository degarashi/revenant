#include "gltf1_scene.hpp"
#include "../../dir.hpp"
#include "../../uri.hpp"
#include "../../gltf/v1/mgr.hpp"
#include "../../gltf/v1/dc_model.hpp"
#include "../../debug_gui/resource_window.hpp"
#include "../../debug_gui/window.hpp"
#include "../../debug_gui/id.hpp"
#include "../../imgui/imgui.h"
#include "../../dc/node.hpp"
#include "../../camera3d.hpp"
#include "../../u_matrix3d.hpp"
#include "../../glx_if.hpp"

namespace rev::test {
	glTF1Scene::glTF1Scene() {
		setStateNew<St_Default>();
		_makeGui();
	}
	void glTF1Scene::_loadModel(const std::string& path) const {
		_cameraIndex = 0;
		auto g = mgr_gltf.loadGLTf(FileURI(path));
		auto& dsc = g->defaultScene;
		D_Assert0(dsc);
		{
			{
				auto sc = *(*dsc);
				// モデル
				_model = gltf::v1::GLTFModel::FromScene(sc);
				// カメラリスト
				gltf::NodeVisitor_Camera<gltf::v1::Node> c;
				for(auto& n : sc.node) {
					n->visit(c);
				}
				_camera = c.result();
				std::sort(_camera.begin(), _camera.end(), [](const auto& e0, const auto& e1){
					return e0.name < e1.name;
				});
			}
			// アニメーション
			_anim.clear();
			for(auto& a : g->m_Animation)
				_anim.append(a.second->makeAnimation());
		}
		// モデル詳細GUIを表示
		debug::ResourceWindow::Add(_model);
	}
	struct glTF1Scene::St_Default : StateT<St_Default, St_Base> {
		void onEnter(glTF1Scene& self, const ObjTypeId_OP id) override {
			St_Base::onEnter(self, id);

			// サンプルglTFファイルを列挙
			self._fileFullPath = Dir::EnumEntryWildCard(Dir::GetProgramDir() + "/resource/gltf_sample/1.0/*/glTF/*.gltf");
			std::sort(self._fileFullPath.begin(), self._fileFullPath.end());
			const std::size_t len = self._fileFullPath.size();
			self._fileList.resize(len);
			for(std::size_t i=0 ; i<len ; i++) {
				const PathBlock pb(self._fileFullPath[i]);
				self._fileList[i] = pb.getLast_utf8();
			}
		}
		void onUpdate(glTF1Scene& self) override {
			St_Base::onUpdate(self);
			if(self._cameraIndex == 0)
				self._fpc.update();
		}
		void onDraw(const glTF1Scene& self, IEffect& e) const override {
			St_Base::onDraw(self, e);
			// モデルセレクタ
			if(const auto _ = debug::WindowPush("model select")) {
				auto* st = ImGui::GetStateStorage();
				int* sel = st->GetIntRef(ImGui::GetID(""), -1);
				int idx=0;
				for(auto& f : self._fileList) {
					const auto _ = debug::IdPush(idx);
					if(ImGui::Selectable(f.c_str(), idx==*sel)) {
						*sel = idx;
						self._dirtyFlag = true;
					}
					++idx;
				}
				if(self._dirtyFlag) {
					self._dirtyFlag = false;
					if(*sel >= 0) {
						self._loadModel(self._fileFullPath[*sel]);
					}
				}
			}
			// カメラセレクタ
			if(self._model) {
				auto& idToNode = self._model->getNode()->getIdToNode();
				if(const auto _ = debug::WindowPush("camera select")) {
					std::size_t idx=0;
					if(ImGui::Selectable("User", self._cameraIndex==idx)) {
						self._cameraIndex = idx;
					}
					++idx;
					for(auto& c : self._camera) {
						const auto _ = debug::IdPush(idx);
						if(ImGui::Selectable(c.name.c_str(), self._cameraIndex==idx)) {
							self._cameraIndex = idx;
						}
						++idx;
					}
				}
				auto& e3 = dynamic_cast<U_Matrix3D&>(e);
				if(self._cameraIndex > 0) {
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
					e3.setCamera(info.camera);
				} else {
					e3.setCamera(self._fpc.getCamera());
				}
			}
			if(self._model)
				self._model->draw(e);
			if(!self._anim.empty()) {
				auto& node = *self._model->getNode();
				constexpr float dt = 1.f / 60;
				if(self._anim.update(node, dt))
					self._anim.loop();
			}
		}
	};
}
