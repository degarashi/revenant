#include "mainthread.hpp"
#include "watch.hpp"
#include "apppath.hpp"

namespace rev {
	void MainThread::_setupFxNotify(FNotify& ntf) {
		const auto flag = FileEvent::Modify|FileEvent::Create|FileEvent::MoveTo;
		{
			mgr_path.enumPath("effect", PathBlock("*"), [&ntf](const Dir& d) {
				if(d.isDirectory()) {
					ntf.addWatch(d.plain_utf8(), flag);
					return false;
				}
				return true;
			});
			auto& path = mgr_path.getPath("effect");
			for(auto& p : path)
				ntf.addWatch(p.plain_utf8(), flag);
		}
	}
}

#include "glx.hpp"
#include "techmgr.hpp"
namespace rev {
	void MainThread::_checkFxReload(FNotify& ntf) {
		using PathSet = std::unordered_set<Dir>;
		PathSet updatePath;
		// シェーダーファイルが更新されていたら再読み込みをかける
		bool bUpdate = false;
		ntf.procEvent([&bUpdate, &ps=updatePath](const FEv& e, const Data_SP&){
			Dir d(*e.basePath);
			d <<= e.name;
			if(d.isFile()) {
				if(ps.emplace(std::move(d)).second)
					bUpdate = true;
			}
		});
		// 連続したファイル操作の直後に再読み込み
		if(!bUpdate && !updatePath.empty()) {
			try {
				for(auto& p : updatePath)
					mgr_tech.setAnonymous(FileURI(p.plain_utf32()));

				// TODO: シェーダーファイルの反映 (仕様が変わったのでEffectの再構築はしない)
				// Effectファイルの再構築(読み込みトライ)
			} catch(const std::exception& e) {
				// 文法エラーが起こったら差し替えない
				Log(Info, "effect reloading failed.\n%s", e.what());
			}
		}
	}
}
