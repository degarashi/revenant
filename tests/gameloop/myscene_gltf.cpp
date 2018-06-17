#include "myscene_gltf.hpp"
#include "../../gltf/v1/mgr.hpp"
#include "../../gltf/v1/scene.hpp"
#include "../../gltf/visitor_camera.hpp"
#include "../../gltf/v1/dc_model.hpp"
#include "../../gltf/v1/animation.hpp"
#include "../../dc/node.hpp"
#include "../../dc/model_if.hpp"
#include "../../debug_gui/resource_window.hpp"
#include "../../debug_gui/window.hpp"
#include "../../debug_gui/print.hpp"
#include "../../imgui/imgui.h"
#include "../../uri.hpp"
#include "../../dir.hpp"
#include "../../u_matrix3d.hpp"
#include "../../glx_if.hpp"
#include "../../camera3d.hpp"
#include "frea/affine_parts.hpp"
#include "frea/yawpitchdist.hpp"
#include <boost/format.hpp>

namespace rev::test {
	using SName = spi::FlyweightItem<std::string>;
	void MyScene::St_glTF::onEnter(MyScene& self, ObjTypeId_OP id) {
		St_3D::onEnter(self, id);
		_cameraBkup = St_3D::_camera;

		// サンプルglTFファイルを列挙
		_fileFullPath = Dir::EnumEntryWildCard(Dir::GetProgramDir() + "/resource/gltf_sample/1.0/*/glTF/*.gltf");
		const std::size_t len = _fileFullPath.size();
		_fileList.resize(len);
		for(std::size_t i=0 ; i<len ; i++) {
			const PathBlock pb(_fileFullPath[i]);
			_fileList[i] = pb.getLast_utf8();
		}
	}
	void MyScene::St_glTF::_loadModel(const std::string& path) const {
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
	void MyScene::St_glTF::onDraw(const MyScene& self, IEffect& e) const {
		St_3D::onDraw(self, e);

		// モデルセレクタ
		if(const auto _ = debug::WindowPush("model select")) {
			auto* st = ImGui::GetStateStorage();
			int* sel = st->GetIntRef(ImGui::GetID(""), -1);
			int idx=0;
			for(auto& f : _fileList) {
				const auto _ = debug::IdPush(idx);
				if(ImGui::Selectable(f.c_str(), idx==*sel)) {
					*sel = idx;
					_dirtyFlag = true;
				}
				++idx;
			}
			if(_dirtyFlag) {
				_dirtyFlag = false;
				if(*sel >= 0) {
					_loadModel(_fileFullPath[*sel]);
				}
			}
		}
		// カメラセレクタ
		if(_model) {
			auto& idToNode = _model->getNode()->getIdToNode();
			if(const auto _ = debug::WindowPush("camera select")) {
				std::size_t idx=0;
				if(ImGui::Selectable("User", _cameraIndex==idx)) {
					_cameraIndex = idx;
					const_cast<St_glTF&>(*this).St_3D::_camera = _cameraBkup;
				}
				++idx;
				for(auto& c : _camera) {
					const auto _ = debug::IdPush(idx);
					if(ImGui::Selectable(c.name.c_str(), _cameraIndex==idx)) {
						_cameraIndex = idx;
					}
					++idx;
				}
			}
			if(_cameraIndex > 0) {
				auto& info = _camera[_cameraIndex-1];
				frea::Mat4 m4 = idToNode.at(info.id)->getTransform();
				m4 *= frea::Mat4::Scaling({1,1,-1});
				beat::g3::Pose pose(m4);
				auto& r = pose.refRotation();
				r.z *= -1;
				r.w *= -1;
				pose.refOffset().z *= -1;
				info.camera->setPose(pose);
				info.camera->setAspect(St_3D::_camera->getAspect());
				const_cast<St_glTF&>(*this).St_3D::_camera = info.camera;
			}
		}
		if(_model)
			_model->draw(e);
		if(!_anim.empty()) {
			auto& node = *_model->getNode();
			constexpr float dt = 1.f / 60;
			if(_anim.update(node, dt))
				_anim.loop();
		}
	}
}
