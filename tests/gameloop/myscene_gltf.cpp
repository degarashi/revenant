#include "myscene.hpp"
#include "../../gltf/mgr.hpp"
#include "../../uri.hpp"
#include "../../gltf/scene.hpp"
#include "../../dc/node.hpp"
#include "../../dc/model.hpp"
#include "../../gltf/visitor_model.hpp"
#include "../../debug_gui/resource_window.hpp"
#include "../../debug_gui/window.hpp"
#include "../../debug_gui/print.hpp"
#include "../../dir.hpp"
#include "../../imgui/imgui.h"
#include "../../gltf/animation.hpp"
#include <boost/format.hpp>

namespace rev::test {
	void MyScene::St_glTF::onEnter(MyScene& self, ObjTypeId_OP id) {
		St_3D::onEnter(self, id);

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
		auto g = mgr_gltf.loadGLTf(FileURI(path));
		auto& dsc = g->defaultScene;
		D_Assert0(dsc);
		{
			auto sc = *(*dsc);
			gltf::Visitor_Model visitor;
			{
				auto node = std::make_shared<dc::TfNode>(0, "", "RootNode");
				node->refPose().identity();
				node->refPose().setScaling(Vec3{1,1,-1});
				visitor.addNode(node);
			}
			for(auto& n : sc->node) {
				n->visit(visitor);
			}
			_model = visitor.result();

			_anim.clear();
			for(auto& a : g->animation.map)
				_anim.emplace(a.first, a.second->makeAnimation());
			_animLen = 0;
			_animCur = 0;
			for(auto& a : _anim)
				_animLen = std::max<float>(_animLen, a.second.length());
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
		if(_model)
			_model->draw(e);
		if(!_anim.empty()) {
			auto& node = *_model->getNode();
			constexpr float dt = 1.f / 60;
			_animCur += dt;
			for(auto& a : _anim) {
				a.second.update(node, dt);
			}
			if(_animCur >= _animLen) {
				for(auto& a : _anim) {
					a.second.reset();
				}
				_animCur = 0;
			}
		}
	}
}
