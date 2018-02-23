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

#include "drawtoken/task.hpp"
#include "glx.hpp"
#include "sharedata.hpp"
#include "glx_block.hpp"
namespace rev {
	void MainThread::_checkFxReload(FNotify& ntf, FxReload& rel) {
		// シェーダーファイルの自動リロード時
		// 描画スレッドが前のシェーダーを実行し終わってから削除する
		if(rel.prevFxCounter > 0) {
			if(--rel.prevFxCounter == 0)
				rel.prevFx.reset();
		}
		// シェーダーファイルが更新されていたら再読み込みをかける
		bool bUpdate = false;
		ntf.procEvent([&bUpdate, &ps=rel.updatePath](const FEv& e, const Data_SP&){
			Dir d(*e.basePath);
			d <<= e.name;
			if(d.isFile()) {
				if(ps.emplace(std::move(d)).second)
					bUpdate = true;
			}
		});
		// 連続したファイル操作の直後に再読み込み
		if(!bUpdate && !rel.updatePath.empty()) {
			try {
				for(auto& p : rel.updatePath)
					mgr_block.setAnonymous(FileURI(p.plain_utf32()));
				rel.updatePath.clear();

				// Effectファイルの再構築(読み込みトライ)
				auto lk = g_system_shared.lock();
				rel.prevFx = rel.curFx;
				rel.curFx = lk->param->makeEffect();
				lk->fx = rel.curFx;
				// ユーザー側で何か移動する変数があればそれをする
				rel.curFx->moveFrom(static_cast<IEffect&>(*rel.prevFx));
				// DrawTaskのバッファ数だけ待ってから削除
				rel.prevFxCounter = draw::Task::NUM_TASK;
			} catch(const std::exception& e) {
				// 文法エラーが起こったら差し替えない
				Log(Info, "effect reloading failed.\n%s", e.what());
			}
		}
	}
}
