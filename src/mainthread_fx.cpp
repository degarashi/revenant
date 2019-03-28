#include "mainthread.hpp"
#include "fs/watch.hpp"
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
		static PathSet updatePath;
		// シェーダーファイルが更新されていたら再読み込みをかける
		bool bUpdate = false;
		ntf.procEvent([&bUpdate](const FEv& e, const Data_SP&){
			Dir d(*e.basePath);
			d <<= e.name;
			if(d.isFile()) {
				if(updatePath.emplace(std::move(d)).second)
					bUpdate = true;
			}
		});
		// 連続したファイル操作の直後に再読み込み
		if(!bUpdate && !updatePath.empty()) {
			for(auto& u : updatePath)
				Log(Verbose, "File changed %s", u.plain_utf8().c_str());
			try {
				// Effectファイルの再構築(読み込みトライ)
				std::vector<std::string> pathv;
				for(auto& p : updatePath)
					pathv.emplace_back(p.plain_utf8());
				mgr_tech.onFileModified(pathv);
			} catch(const std::exception& e) {
				// 文法エラーが起こったら差し替えない
				Log(Error, "effect reloading failed.\n%s", e.what());
			}
			updatePath.clear();
		}
	}
}
